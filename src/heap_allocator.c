#include "heap_allocator.h"

extern size_t parep_mpi_ext_heap_size;
int offset = sizeof(uint64_t);

pthread_mutex_t heap_free_list_mutex = PTHREAD_MUTEX_INITIALIZER;

heap_t parep_mpi_heap;
bin_t parep_mpi_bins[BIN_COUNT];
bin_t parep_mpi_fastbins[FASTBIN_COUNT];

footer_t *get_foot(heap_node_t *node) {
	return (footer_t *)((char *)node + sizeof(heap_node_t) + GET_SIZE(node->hole_and_size));
}

void create_foot (heap_node_t *head) {
	footer_t *foot = get_foot(head);
	foot->header = head;
}

int get_bin_index(size_t sz) {
	int index = 0;
	sz = sz < 4 ? 4: sz;
	
	while(sz = sz >> 1) index++;
	index = index - 2;
	
	if(index > BIN_MAX_IDX) index = BIN_MAX_IDX;
	return index;
}

void add_node(bin_t *bin, heap_node_t *node) {
	node->next = NULL;
	node->prev = NULL;
	
	heap_node_t *temp = bin->head;
	
	if(bin->head == NULL) {
		bin->head = node;
	} else {
		node->next = bin->head;
		bin->head->prev = node;
		bin->head = node;
	}
	
	
	/*heap_node_t *current = bin->head;
	heap_node_t *previous = NULL;
	
	while(current != NULL && GET_SIZE(current->hole_and_size) <= GET_SIZE(node->hole_and_size)) {
		previous = current;
		current = current->next;
	}
	
	if(current == NULL) {
		if(previous != NULL) previous->next = node;
		node->prev = previous;
	} else {
		if(previous != NULL) {
			node->next = current;
			previous->next = node;
			
			node->prev = previous;
			current->prev = node;
		} else {
			node->next = bin->head;
			bin->head->prev = node;
			bin->head = node;
		}
	}*/
}

void remove_node(bin_t *bin, heap_node_t *node) {
	if(bin->head == NULL) return;
	if(bin->head == node) {
		bin->head = bin->head->next;
		if (node->next != NULL) {
			node->next->prev = NULL;
		}
		return;
	}
	
	if(node->next == NULL) {
		node->prev->next = NULL;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	
	/*heap_node_t *temp = bin->head->next;
	while(temp != NULL) {
		if(temp == node) {
			if(temp->next == NULL) {
				temp->prev->next = NULL;
			} else {
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
			}
			return;
		}
		temp = temp->next;
	}*/
}

heap_node_t *get_best_fit(bin_t *bin, size_t size) {
	if(bin->head == NULL) return NULL;
	
	heap_node_t *temp = bin->head;
	while(temp != NULL) {
		if(GET_SIZE(temp->hole_and_size) >= size) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

void parep_mpi_init_heap(address start) {
	heap_node_t *init_region = (heap_node_t *)start;
	SET_HOLE(init_region->hole_and_size,1);
	SET_FAST(init_region->hole_and_size,0);
	SET_SIZE(init_region->hole_and_size,((PAREP_MPI_HEAP_SIZE) - sizeof(heap_node_t) - sizeof(footer_t)));
	
	create_foot(init_region);
	
	for(int i = 0; i < BIN_COUNT; i++) {
		parep_mpi_bins[i].head = NULL;
		parep_mpi_heap.bins[i] = &parep_mpi_bins[i];
	}
	
	for(int i = 0; i < FASTBIN_COUNT; i++) {
		parep_mpi_fastbins[i].head = NULL;
		parep_mpi_heap.fastbins[i] = &parep_mpi_fastbins[i];
	}
	
	add_node(parep_mpi_heap.bins[get_bin_index(GET_SIZE(init_region->hole_and_size))],init_region);
	
	parep_mpi_heap.start = start;
	parep_mpi_heap.end = start + PAREP_MPI_HEAP_SIZE;
}

int expand_heap(size_t sz) {
	if(sz == 0) return 1;
	void *exp_heap = mremap((void *)parep_mpi_heap.start,parep_mpi_heap.end-parep_mpi_heap.start,parep_mpi_heap.end-parep_mpi_heap.start+sz,0);
	if(exp_heap == MAP_FAILED) return 0;
	assert((address)exp_heap == parep_mpi_heap.start);
	//printf("Heap expanded from %p to %p\n",parep_mpi_heap.end-parep_mpi_heap.start,parep_mpi_heap.end-parep_mpi_heap.start+sz);
	heap_node_t *last_node = ((footer_t *)(parep_mpi_heap.end - sizeof(footer_t)))->header;
	
	if(GET_HOLE(last_node->hole_and_size)) {
		//This is free node. Just expand it.
		bin_t *list = parep_mpi_heap.bins[get_bin_index(GET_SIZE(last_node->hole_and_size))];
		remove_node(list,last_node);
		
		size_t last_node_size = GET_SIZE(last_node->hole_and_size);
		SET_SIZE(last_node->hole_and_size, ((last_node_size) + (sz)));
		create_foot(last_node);
		
		add_node(parep_mpi_heap.bins[get_bin_index(GET_SIZE(last_node->hole_and_size))],last_node);
	} else {
		//Node has been allocated. Create new node.
		heap_node_t *init_region = (heap_node_t *)(parep_mpi_heap.end);
		SET_HOLE(init_region->hole_and_size,1);
		SET_FAST(init_region->hole_and_size,0);
		SET_SIZE(init_region->hole_and_size,((sz) - sizeof(heap_node_t) - sizeof(footer_t)));
		
		create_foot(init_region);
		add_node(parep_mpi_heap.bins[get_bin_index(GET_SIZE(init_region->hole_and_size))],init_region);
	}
	parep_mpi_heap.end = parep_mpi_heap.end + sz;
	parep_mpi_ext_heap_size = parep_mpi_heap.end - parep_mpi_heap.start;
	return 1;
}

void *parep_mpi_malloc(size_t size) {
	if(size == 0) return NULL;
	
	size = size & ~7; //Round down to nearest 8-byte boundary
	
	pthread_mutex_lock(&heap_free_list_mutex);
	
	//FASTBIN CODE START
	if(size <= FASTBIN_SIZE_THRESHOLD) {
		int fastbin_index = size / 8;
		bin_t *temp = (bin_t *)parep_mpi_heap.fastbins[fastbin_index];
		heap_node_t *found = temp->head;
		if(found != NULL) {
			SET_FAST(found->hole_and_size,0);
			temp->head = temp->head->next;
			found->prev = NULL;
			found->next = NULL;
			pthread_mutex_unlock(&heap_free_list_mutex);
			return &found->next;
		}
	}
	//FASTBIN CODE END
	
	//LARGE MALLOC CODE START
	if(size >= MIN_LARGE_SIZE) {
		parep_mpi_malloc_consolidate();
	}
	//LARGE MALLOC CODE END
	
	int index = get_bin_index(size);
	int base_index = index;
	bin_t *temp = (bin_t *)parep_mpi_heap.bins[index];
	heap_node_t *found = get_best_fit(temp,size);
	
	bool consolidate_attempted = false;
	while(found == NULL) {
		if(index+1 >= BIN_COUNT) {
			if(!consolidate_attempted) {
				parep_mpi_malloc_consolidate();
				consolidate_attempted = true;
				index = get_bin_index(size);
				temp = parep_mpi_heap.bins[index];
				found = get_best_fit(temp,size);
				continue;
			}
			if((parep_mpi_heap.end - parep_mpi_heap.start) >= PAREP_MPI_HEAP_MAX_SIZE)  {
				pthread_mutex_unlock(&heap_free_list_mutex);
				return NULL;
			} else {
				//Double heap size every expansion
				int success;
				if((parep_mpi_heap.end - parep_mpi_heap.start) < 0x80000000) {
					success = expand_heap(parep_mpi_heap.end - parep_mpi_heap.start);
				} else {
					success = expand_heap(0x40000000);
				}
				if(success == 0) {
					pthread_mutex_unlock(&heap_free_list_mutex);
					return NULL;
				}
				index = base_index-1;
			}
		}
		temp = parep_mpi_heap.bins[++index];
		found = get_best_fit(temp,size);
	}
	
	if(((GET_SIZE(found->hole_and_size)) - size) > (overhead + MIN_ALLOC_SZ)) {
		heap_node_t *split = (heap_node_t *)(((char *)found + overhead) + size);
		size_t foundsize = GET_SIZE(found->hole_and_size);
		SET_SIZE(split->hole_and_size, ((foundsize) - (size) - (overhead)));
		SET_HOLE(split->hole_and_size,1);
		SET_FAST(split->hole_and_size,0);
		
		create_foot(split);
		
		int new_idx = get_bin_index(GET_SIZE(split->hole_and_size));
		add_node(parep_mpi_heap.bins[new_idx],split);
		
		SET_SIZE(found->hole_and_size,size);
		create_foot(found);
	}
	
	SET_HOLE(found->hole_and_size,0);
	SET_FAST(found->hole_and_size,0);
	remove_node(parep_mpi_heap.bins[index],found);
	
	pthread_mutex_unlock(&heap_free_list_mutex);
	
	found->prev = NULL;
	found->next = NULL;
	return &found->next;
}

void parep_mpi_free(void *p) {
	if(p == NULL) return;
	pthread_mutex_lock(&heap_free_list_mutex);
	bin_t *list;
	footer_t *new_foot, *old_foot;
	
	heap_node_t *head = (heap_node_t *)((char *)p - offset);
	
	//FASTBIN CODE START
	if(GET_SIZE(head->hole_and_size) <= FASTBIN_SIZE_THRESHOLD) {
		int fastbin_index = GET_SIZE(head->hole_and_size) / 8;
		bin_t *temp = (bin_t *)parep_mpi_heap.fastbins[fastbin_index];
		SET_FAST(head->hole_and_size,1);
		head->next = temp->head;
		temp->head = head;
		pthread_mutex_unlock(&heap_free_list_mutex);
		return;
	}
	//FASTBIN CODE END
	
	heap_node_t *next = (heap_node_t *)((char *)get_foot(head) + sizeof(footer_t));
	heap_node_t *prev;
	if((address)head != parep_mpi_heap.start) {
		prev = ((footer_t *)((char *)head - sizeof(footer_t)))->header;
		if(GET_HOLE(prev->hole_and_size)) {
			list = parep_mpi_heap.bins[get_bin_index(GET_SIZE(prev->hole_and_size))];
			remove_node(list,prev);
			
			size_t headsize = GET_SIZE(head->hole_and_size);
			size_t prevsize = GET_SIZE(prev->hole_and_size);
			SET_SIZE(prev->hole_and_size, ((prevsize) + (overhead) + (headsize)));
			create_foot(prev);
			
			head = prev;
		}
	}
	if((address)next != parep_mpi_heap.end) {
		if(GET_HOLE(next->hole_and_size)) {
			list = parep_mpi_heap.bins[get_bin_index(GET_SIZE(next->hole_and_size))];
			remove_node(list,next);
			
			size_t headsize = GET_SIZE(head->hole_and_size);
			size_t nextsize = GET_SIZE(next->hole_and_size);
			SET_SIZE(head->hole_and_size, ((headsize) + (overhead) + (nextsize)));
			
			old_foot = get_foot(next);
			old_foot->header = 0;
			SET_SIZE(next->hole_and_size,0);
			SET_HOLE(next->hole_and_size,0);
			SET_FAST(next->hole_and_size,0);
			
			create_foot(head);
		}
	}
	
	SET_HOLE(head->hole_and_size,1);
	SET_FAST(head->hole_and_size,0);
	add_node(parep_mpi_heap.bins[get_bin_index(GET_SIZE(head->hole_and_size))],head);
	
	if(GET_SIZE(head->hole_and_size) >= FASTBIN_CONSOLIDATION_THRESHOLD) {
		parep_mpi_malloc_consolidate();
	}
	
	pthread_mutex_unlock(&heap_free_list_mutex);
}

void *parep_mpi_realloc(void *old_ptr,size_t new_size) {
	if(old_ptr == NULL) {
		return parep_mpi_malloc(new_size);
	} else if(new_size == 0) {
		parep_mpi_free(old_ptr);
		return NULL;
	} else {
		void *reallocated = NULL;
		reallocated = parep_mpi_malloc(new_size);
		memcpy(reallocated,old_ptr,new_size);
		parep_mpi_free(old_ptr);
		return reallocated;
	}
}

void *parep_mpi_calloc(size_t nmemb, size_t size) {
	void *retval;
	retval = parep_mpi_malloc(nmemb*size);
	memset(retval,0,nmemb*size);
	return retval;
}
char *parep_mpi_strdup(const char *str) {
	size_t len;
	char *copy;
	len = strlen(str) + 1;
	copy = parep_mpi_malloc(len);
	if(copy == NULL) return NULL;
	memcpy(copy,str,len);
	return copy;
}
char *parep_mpi_strndup(const char *str, size_t n) {
	size_t len;
	char *copy;
	
	for(len = 0; len < n && str[len]; len++) continue;
	
	copy = parep_mpi_malloc(len+1);
	if(copy == NULL) return NULL;
	memcpy(copy,str,len);
	copy[len] = '\0';
	return copy;
}

static int resolve_path(char *path,char *result,char *pos) {
	if (*path == '/') {
		*result = '/';
		pos = result+1;
		path++;
	}
	*pos = 0;
	if (!*path) return 0;
	while (1) {
		char *slash;
		struct stat st;
		slash = *path ? strchr(path,'/') : NULL;
		if (slash) *slash = 0;
		if (!path[0] || (path[0] == '.' && (!path[1] || (path[1] == '.' && !path[2])))) {
			pos--;
			if (pos != result && path[0] && path[1]) while (*--pos != '/');
		} else {
			strcpy(pos,path);
			if (lstat(result,&st) < 0) return -1;
			if (S_ISLNK(st.st_mode)) {
				char buf[PATH_MAX];
				if (readlink(result,buf,sizeof(buf)) < 0) return -1;
				*pos = 0;
				if (slash) {
					*slash = '/';
					strcat(buf,slash);
				}
				strcpy(path,buf);
				if (*path == '/') result[1] = 0;
				pos = strchr(result,0);
				continue;
			}
			pos = strchr(result,0);
		}
		if (slash) {
			*pos++ = '/';
			path = slash+1;
		}
		*pos = 0;
		if (!slash) break;
	}
	return 0;
}

char *parep_mpi_realpath(const char *path, char *resolved_path) {
	char cwd[PATH_MAX];
	char *path_copy;
	int res;
	if (!*path) {
		errno = ENOENT;
		return NULL;
	}
	if (!getcwd(cwd,sizeof(cwd))) return NULL;
	strcpy(resolved_path,"/");
	if(resolve_path(cwd,resolved_path,resolved_path)) return NULL;
	strcat(resolved_path,"/");
	path_copy = parep_mpi_strdup(path);
	if(!path_copy) return NULL;
	res = resolve_path(path_copy,resolved_path,strchr(resolved_path,0));
	parep_mpi_free(path_copy);
	if(res) return NULL;
	return resolved_path;
}

void parep_mpi_malloc_consolidate(void) {
	//pthread_mutex_lock(&heap_free_list_mutex);
	for(int i = 0; i < FASTBIN_COUNT; i++) {
		bin_t *temp = (bin_t *)parep_mpi_heap.fastbins[i];
		while(temp->head != NULL) {
			heap_node_t *head = temp->head;
			temp->head = temp->head->next;
			
			bin_t *list;
			footer_t *new_foot, *old_foot;
			
			heap_node_t *next = (heap_node_t *)((char *)get_foot(head) + sizeof(footer_t));
			heap_node_t *prev;
			if((address)head != parep_mpi_heap.start) {
				prev = ((footer_t *)((char *)head - sizeof(footer_t)))->header;
				if(GET_HOLE(prev->hole_and_size)) {
					list = parep_mpi_heap.bins[get_bin_index(GET_SIZE(prev->hole_and_size))];
					remove_node(list,prev);
					
					size_t headsize = GET_SIZE(head->hole_and_size);
					size_t prevsize = GET_SIZE(prev->hole_and_size);
					SET_SIZE(prev->hole_and_size, ((prevsize) + (overhead) + (headsize)));
					create_foot(prev);
					
					head = prev;
				}
			}
			if((address)next != parep_mpi_heap.end) {
				if(GET_HOLE(next->hole_and_size)) {
					list = parep_mpi_heap.bins[get_bin_index(GET_SIZE(next->hole_and_size))];
					remove_node(list,next);
					
					size_t headsize = GET_SIZE(head->hole_and_size);
					size_t nextsize = GET_SIZE(next->hole_and_size);
					SET_SIZE(head->hole_and_size, ((headsize) + (overhead) + (nextsize)));
					
					old_foot = get_foot(next);
					old_foot->header = 0;
					SET_SIZE(next->hole_and_size,0);
					SET_HOLE(next->hole_and_size,0);
					SET_FAST(next->hole_and_size,0);
					
					create_foot(head);
				}
			}
	
			SET_HOLE(head->hole_and_size,1);
			SET_FAST(head->hole_and_size,0);
			add_node(parep_mpi_heap.bins[get_bin_index(GET_SIZE(head->hole_and_size))],head);
		}
	}
	//pthread_mutex_unlock(&heap_free_list_mutex);
}
#include "heap_allocator.h"
#include "commbuf_cache.h"

commbuf_bin_t parep_mpi_commbuf_bins[PAREP_MPI_COMMBUF_BIN_COUNT];

int parep_mpi_num_commbuf_nodes = 0;

int get_commbuf_bin_index(size_t sz) {
	if (sz <= 8) {
		return 0;
	}
	unsigned int lower_bound = 8;
	int index = 1;
	
	while(index < PAREP_MPI_COMMBUF_BIN_COUNT) {
		unsigned int upper_bound = lower_bound << 2;
		if (lower_bound < sz && sz <= upper_bound) {
			return index;
		}
		lower_bound = upper_bound;
		index++;
	}
	return PAREP_MPI_COMMBUF_BIN_MAX_IDX;
}

commbuf_node_t *new_commbuf_node(size_t size) {
	commbuf_node_t *ret = parep_mpi_malloc(sizeof(commbuf_node_t));
	ret->commbuf = parep_mpi_malloc(size);
	ret->size = size;
	ret->next = NULL;
	ret->prev = NULL;
	return ret;
}

void delete_commbuf_node(commbuf_node_t *node) {
	parep_mpi_free(node->commbuf);
	parep_mpi_free(node);
}

int commbuf_node_insert(commbuf_node_t *node,commbuf_bin_t *bin) {
	if(parep_mpi_num_commbuf_nodes >= PAREP_MPI_COMMBUF_CACHE_ENTRIES) {
		return -1;
	}
	if(bin->head == NULL) {
		bin->tail = node;
	} else {
		node->next = bin->head;
		bin->head->prev = node;
	}
	bin->head = node;
	parep_mpi_num_commbuf_nodes++;
	return 0;
}

int commbuf_node_reinsert(commbuf_node_t *node,commbuf_bin_t *bin) {
	if(parep_mpi_num_commbuf_nodes >= PAREP_MPI_COMMBUF_CACHE_ENTRIES_MAX) {
		return -1;
	}
	if(bin->head == NULL) {
		bin->tail = node;
	} else {
		node->next = bin->head;
		bin->head->prev = node;
	}
	bin->head = node;
	parep_mpi_num_commbuf_nodes++;
	return 0;
}

commbuf_node_t *commbuf_node_remove(commbuf_node_t *node,commbuf_bin_t *bin) {
	if(node->prev != NULL) node->prev->next = node->next;
	else bin->head = node->next;
	if(node->next != NULL) node->next->prev = node->prev;
	else bin->tail = node->prev;
	parep_mpi_num_commbuf_nodes--;
	return node;
}

commbuf_node_t *commbuf_node_find(size_t size) {
	commbuf_node_t *ret = NULL;
	commbuf_node_t *temp;
	int bin_index = get_commbuf_bin_index(size);
	temp = parep_mpi_commbuf_bins[bin_index].head;
	while(temp != NULL) {
		if(size <= temp->size) {
			ret = temp;
			break;
		}
		temp = temp->next;
	}
	return ret;
}

commbuf_node_t *commbuf_node_evict(commbuf_bin_t *bin) {
	commbuf_node_t *ret = bin->tail;
	if(ret == NULL) return ret;
	if(ret->prev != NULL) ret->prev->next = ret->next;
	else bin->head = ret->next;
	bin->tail = ret->prev;
	parep_mpi_num_commbuf_nodes--;
	return ret;
}

commbuf_node_t *get_commbuf_node(size_t size) {
	commbuf_node_t *ret = commbuf_node_find(size);
	if(ret == NULL) {
		ret = new_commbuf_node(size);
		int evict_index = get_commbuf_bin_index(ret->size);
		commbuf_bin_t *bin = &parep_mpi_commbuf_bins[evict_index];
		while(commbuf_node_insert(ret,bin) < 0) {
			commbuf_node_t *evicted = commbuf_node_evict(&parep_mpi_commbuf_bins[evict_index]);
			if(evicted == NULL) evict_index = (evict_index - 1 < 0) ? PAREP_MPI_COMMBUF_BIN_MAX_IDX : (evict_index - 1);
			else delete_commbuf_node(evicted);
		}
		ret = commbuf_node_remove(ret,bin);
		ret->next = NULL;
		ret->prev = NULL;
	} else {
		commbuf_bin_t *bin = &parep_mpi_commbuf_bins[get_commbuf_bin_index(ret->size)];
		ret = commbuf_node_remove(ret,bin);
		ret->next = NULL;
		ret->prev = NULL;
	}
	return ret;
}

void return_commbuf_node(commbuf_node_t *node) {
	memset(node->commbuf,0,node->size);
	int evict_index = get_commbuf_bin_index(node->size);
	commbuf_bin_t *bin = &parep_mpi_commbuf_bins[evict_index];
	while(commbuf_node_reinsert(node,bin) < 0) {
		commbuf_node_t *evicted = commbuf_node_evict(&parep_mpi_commbuf_bins[evict_index]);
		if(evicted == NULL) evict_index = (evict_index - 1 < 0) ? PAREP_MPI_COMMBUF_BIN_MAX_IDX : (evict_index - 1);
		else delete_commbuf_node(evicted);
	}
}
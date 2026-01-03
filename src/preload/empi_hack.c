#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <limits.h>
#include <poll.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define BUFFER_SIZE 256
#define PAREP_MPI_COORDINATOR_PORT 2581
#define CMD_WAITPID_OUTPUT 7
#define CMD_WAITPID_DATA 14
#define CMD_MISC_PROC_FAILED 15
#define CMD_MPI_INITIALIZED 18

struct waitpid_output {
	pid_t pid;
	int status;
	struct waitpid_output *next;
};
typedef struct waitpid_output wpid_out;

struct internal_pollfds {
	struct pollfd *user_pfds;
	struct pollfd *real_pfds;
	int num_active_fds;
	struct internal_pollfds *next;
};
typedef struct internal_pollfds internal_pfds;

internal_pfds *pfd_head = NULL;
internal_pfds *pfd_tail = NULL;

wpid_out *head = NULL;
wpid_out *tail = NULL;

int (*real_poll)(struct pollfd *, nfds_t, int);
ssize_t (*real_read)(int,void *,size_t);
pid_t (*real_waitpid)(pid_t, int *, int) = NULL;
int (*real_sigaction)(int signum, const struct sigaction *__restrict act, struct sigaction *__restrict oldact) = NULL;
sighandler_t (*real_signal)(int signum, sighandler_t handler);

struct sockaddr_un server_addr;
bool server_addr_set = false;
pthread_mutex_t serv_addr_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t serv_addr_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_coordinator_sock;

pthread_t coordinator_poller;

int check(int exp, const char *msg) {
	if (exp == -1) {
		perror(msg);
		exit(1);
	}
	return exp;
}

int pollintrpipe[2];
int safe_to_pass = 0;
pthread_mutex_t safe_to_pass_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t safe_to_pass_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_node_num;
int parep_mpi_initialized = 0;
pthread_mutex_t parep_mpi_initialized_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_initialized_cond = PTHREAD_COND_INITIALIZER;

void *poll_coordinator(void *arg) {
	nfds_t nfds;
	struct pollfd pfd;
	pfd.fd = parep_mpi_coordinator_sock;
	pfd.events = POLLIN;
	nfds = 1;
	int pollret;
	while(true) {
		pollret = real_poll(&pfd,nfds,-1);
		if((pollret == -1) && (errno == EINTR)) continue;
		if (pfd.revents != 0) {
			if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR)) {
				printf("EMPI code: POLLUP/POLLERR on coordinator socket POLLIN mpiexec %d\n",(pfd.revents & POLLIN));
				fflush(stdout);
				while(1);
				exit(0);
				pthread_mutex_lock(&safe_to_pass_mutex);
				safe_to_pass = 1;
				pthread_cond_broadcast(&safe_to_pass_cond);
				pthread_mutex_unlock(&safe_to_pass_mutex);
				int intr = 1;
				write(pollintrpipe[1],&intr,sizeof(int));
				close(parep_mpi_coordinator_sock);
				pthread_exit(NULL);
			}
			if(pfd.revents & POLLIN) {
				size_t bytes_read;
				int msgsize = 0;
				char buffer[BUFFER_SIZE];
				while((bytes_read = real_read(parep_mpi_coordinator_sock,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				check(bytes_read,"recv error safe to pass mpiexec");
				int cmd = *((int *)buffer);
				if(cmd == CMD_MPI_INITIALIZED) {
					pthread_mutex_lock(&parep_mpi_initialized_mutex);
					parep_mpi_initialized = 1;
					pthread_cond_broadcast(&parep_mpi_initialized_cond);
					pthread_mutex_unlock(&parep_mpi_initialized_mutex);
				} else {
					pthread_mutex_lock(&safe_to_pass_mutex);
					safe_to_pass = *((int *)buffer);
					safe_to_pass = safe_to_pass > 0;
					pthread_cond_broadcast(&safe_to_pass_cond);
					pthread_mutex_unlock(&safe_to_pass_mutex);
					if(safe_to_pass == 1) {
						int intr = 1;
						write(pollintrpipe[1],&intr,sizeof(int));
						close(parep_mpi_coordinator_sock);
						pthread_exit(NULL);
					}
				}
			}
		}
	}
}

int send_to_server(pid_t *pid_buf, int num_pid, int wnohang_used) {
	char buffer[BUFFER_SIZE];
	size_t bytes_read;
	int msgsize = 0;
	
	int cmd = CMD_WAITPID_DATA;
	*((int *)buffer) = cmd;
	write(parep_mpi_coordinator_sock,buffer,sizeof(int));
	*((int *)buffer) = wnohang_used;
	write(parep_mpi_coordinator_sock,buffer,sizeof(int));
	*((int *)buffer) = num_pid;
	write(parep_mpi_coordinator_sock,buffer,sizeof(int));
	if(num_pid > 0) write(parep_mpi_coordinator_sock,pid_buf,sizeof(pid_t) * num_pid);
	
	int stp;
	if(wnohang_used) {
		pthread_mutex_lock(&safe_to_pass_mutex);
		stp = safe_to_pass;
		pthread_mutex_unlock(&safe_to_pass_mutex);
	} else {
		if(num_pid > 0) {
			pthread_mutex_lock(&safe_to_pass_mutex);
			stp = safe_to_pass;
			pthread_mutex_unlock(&safe_to_pass_mutex);
		} else {
			pthread_mutex_lock(&safe_to_pass_mutex);
			while(safe_to_pass == 0) {
				pthread_cond_wait(&safe_to_pass_cond,&safe_to_pass_mutex);
			}
			stp = safe_to_pass;
			pthread_mutex_unlock(&safe_to_pass_mutex);
			assert(stp == 1);
		}
	}
	return stp;
}

/*int sigaction(int signum, const struct sigaction *__restrict act, struct sigaction *__restrict oldact) {
	if(real_sigaction == NULL) real_sigaction = dlsym(RTLD_NEXT,"sigaction");
	//if(signum == SIGCHLD) printf("%d: SIGACTION CALLED FOR SIGCHLD\n",getpid());
	return real_sigaction(signum,act,oldact);
}

sighandler_t signal(int signum, sighandler_t handler) {
	if(real_signal == NULL) real_signal = dlsym(RTLD_NEXT,"signal");
	//if(signum == SIGCHLD) printf("%d: SIGNAL CALLED FOR SIGCHLD\n",getpid());
	return real_signal(signum,handler);
}*/

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
	if(real_waitpid == NULL) {
		real_waitpid = dlsym(RTLD_NEXT,"waitpid");
	}
	if(real_poll == NULL) {
		real_poll = dlsym(RTLD_NEXT,"poll");
	}
	if(real_read == NULL) {
		real_read = dlsym(RTLD_NEXT,"read");
	}
	if(!getenv("PAREP_MPI_PROXY_HACKED") || strcmp(getenv("PAREP_MPI_PROXY_HACKED"),"1")) return real_poll(fds,nfds,timeout);
	
	pthread_mutex_lock(&serv_addr_mutex);
	if(!server_addr_set) {
		parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
		memset(&server_addr,0,sizeof(server_addr));
		server_addr.sun_family = AF_UNIX;
		strcpy(server_addr.sun_path,"#EMPIServermpiexec");
		server_addr.sun_path[0] = 0;
		
		check((parep_mpi_coordinator_sock = socket(AF_UNIX, SOCK_STREAM, 0)),"Failed to create socket mpiexec");
		int ret = connect(parep_mpi_coordinator_sock,(struct sockaddr *)(&server_addr),sizeof(server_addr));
		while(ret != 0) {
			ret = connect(parep_mpi_coordinator_sock,(struct sockaddr *)(&server_addr),sizeof(server_addr));
		}
		if(pipe(pollintrpipe) < 0) {
			perror("Pollintrpipe creation failed mpiexec\n");
			exit(1);
		}
		server_addr_set = true;
		
		pid_t mypid = getpid();
		write(parep_mpi_coordinator_sock,&mypid,sizeof(pid_t));
		
		pthread_create(&coordinator_poller,NULL,poll_coordinator,NULL);
	}
	pthread_mutex_unlock(&serv_addr_mutex);
	
	int pre_errno = errno;
	int infinite_timeout = (timeout < 0);
	int num_active_fds = nfds;
	for(int i = 0; i < nfds; i++) {
		if(fds[i].fd < 0) num_active_fds--;
		fds[i].revents = 0;
	}
	int pollret;
	int has_bad_sock = 0;
	internal_pfds *current_ipfd;
	
	struct pollfd *real_fds = fds;
	
	pthread_mutex_lock(&safe_to_pass_mutex);
	if(!safe_to_pass) {
		if(pfd_head != NULL) {
			internal_pfds *temp = pfd_head;
			while(temp != NULL) {
				if(temp->user_pfds == fds) {
					real_fds = temp->real_pfds;
					num_active_fds = temp->num_active_fds;
					has_bad_sock = 1;
					current_ipfd = temp;
					break;
				}
				temp = temp->next;
			}
		}
	}
	if((timeout >= 0) || (num_active_fds > 0) || (safe_to_pass)) {
		if((infinite_timeout) && (has_bad_sock)) {
			real_fds[nfds].fd = pollintrpipe[0];
			real_fds[nfds].events = POLLIN;
			pollret = 0;
			do {
				pthread_mutex_unlock(&safe_to_pass_mutex);
				pollret = real_poll(real_fds,nfds+1,timeout);
				pthread_mutex_lock(&safe_to_pass_mutex);
				for(int i = 0; i < nfds; i++) {
					if(real_fds[i].revents != 0) {
						fds[i].revents |= real_fds[i].revents;
					}
				}
				if(real_fds[nfds].revents != 0) {
					if(real_fds[nfds].revents & POLLIN) {
						int intr = 0;
						real_read(pollintrpipe[0],&intr,sizeof(int));
						assert(intr == 1);
						close(pollintrpipe[0]);
						close(pollintrpipe[1]);
						current_ipfd->num_active_fds = nfds;
						for(int i = 0; i < nfds; i++) {
							if(fds[i].fd < 0) current_ipfd->num_active_fds--;
							real_fds[i].fd = fds[i].fd;
							real_fds[i].events = fds[i].events;
						}
						pthread_mutex_unlock(&safe_to_pass_mutex);
						pollret = real_poll(real_fds,nfds,timeout);
						pthread_mutex_lock(&safe_to_pass_mutex);
						break;
					}
					pollret--;
				}
			} while(pollret == 0);
		} else {
			pthread_mutex_unlock(&safe_to_pass_mutex);
			pollret = real_poll(real_fds,nfds,timeout);
			pthread_mutex_lock(&safe_to_pass_mutex);
		}
		if(pollret <= 0) {
			pthread_mutex_unlock(&safe_to_pass_mutex);
			return pollret;
		}
		int bad_sock_detected = 0;
		for(int i = 0; i < nfds; i++) {
			if(real_fds[i].revents != 0) {
				fds[i].revents |= real_fds[i].revents;
				if((real_fds[i].revents & POLLHUP) || (real_fds[i].revents & POLLERR) || ((i >= nfds-parep_mpi_node_num) && (real_fds[i].revents & POLLIN) && (parep_mpi_initialized))) {
					bad_sock_detected = 1;
				}
			}
		}
		if(bad_sock_detected == 0) {
			pthread_mutex_unlock(&safe_to_pass_mutex);
			return pollret;
		}
		if(safe_to_pass) {
			pthread_mutex_unlock(&safe_to_pass_mutex);
			return pollret;
		}
		pthread_mutex_lock(&parep_mpi_initialized_mutex);
		if(parep_mpi_initialized == 0) {
			pthread_mutex_unlock(&parep_mpi_initialized_mutex);
			pthread_mutex_unlock(&safe_to_pass_mutex);
			return pollret;
		}
		pthread_mutex_unlock(&parep_mpi_initialized_mutex);
	}
	pthread_mutex_unlock(&safe_to_pass_mutex);
	
	if(!has_bad_sock) {
		internal_pfds *ipfds = (internal_pfds *)malloc(sizeof(internal_pfds));
		ipfds->user_pfds = fds;
		ipfds->real_pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * (nfds+1));
		ipfds->num_active_fds = num_active_fds;
		ipfds->next = NULL;
		if(pfd_tail == NULL) {
			pfd_head = ipfds;
		} else {
			pfd_tail->next = ipfds;
		}
		pfd_tail = ipfds;
		real_fds = ipfds->real_pfds;
		current_ipfd = ipfds;
	}
	
	int cmd = CMD_MISC_PROC_FAILED;
	write(parep_mpi_coordinator_sock,&cmd,sizeof(int));
	write(parep_mpi_coordinator_sock,&infinite_timeout,sizeof(int));
	write(parep_mpi_coordinator_sock,&num_active_fds,sizeof(int));
	
	if(!infinite_timeout) {
		pthread_mutex_lock(&safe_to_pass_mutex);
		if(!safe_to_pass) {
			for(int i = 0; i < nfds; i++) {
				if(!has_bad_sock) {
					real_fds[i].fd = fds[i].fd;
					real_fds[i].events = fds[i].events;
				}
				if(fds[i].revents != 0) {
					if((fds[i].revents & POLLHUP) || (fds[i].revents & POLLERR) || ((i >= nfds-parep_mpi_node_num) && (fds[i].revents & POLLIN) && (parep_mpi_initialized))) {
						real_fds[i].fd = -1;
						fds[i].revents = 0;
						pollret--;
						current_ipfd->num_active_fds--;
					}
				}
			}
			if(!has_bad_sock) has_bad_sock = 1;
		} else {
			current_ipfd->num_active_fds = nfds;
			for(int i = 0; i < nfds; i++) {
				if(fds[i].fd < 0) current_ipfd->num_active_fds--;
				real_fds[i].fd = fds[i].fd;
				real_fds[i].events = fds[i].events;
			}
		}
		pthread_mutex_unlock(&safe_to_pass_mutex);
		return pollret;
	} else {
		do {
			if(num_active_fds == 0) {
				pthread_mutex_lock(&safe_to_pass_mutex);
				while(safe_to_pass == 0) {
					pthread_cond_wait(&safe_to_pass_cond,&safe_to_pass_mutex);
				}
				pthread_mutex_unlock(&safe_to_pass_mutex);
				assert(safe_to_pass == 1);
				current_ipfd->num_active_fds = nfds;
				for(int i = 0; i < nfds; i++) {
					if(fds[i].fd < 0) current_ipfd->num_active_fds--;
					fds[i].revents |= real_fds[i].revents;
					real_fds[i].fd = fds[i].fd;
					real_fds[i].events = fds[i].events;
				}
				pollret = real_poll(fds,nfds,timeout);
				return pollret;
			} else {
				pthread_mutex_lock(&safe_to_pass_mutex);
				if(!safe_to_pass) {
					for(int i = 0; i < nfds; i++) {
						if(!has_bad_sock) {
							real_fds[i].fd = fds[i].fd;
							real_fds[i].events = fds[i].events;
						}
						if(fds[i].revents != 0) {
							if((fds[i].revents & POLLHUP) || (fds[i].revents & POLLERR) || ((i >= nfds-parep_mpi_node_num) && (fds[i].revents & POLLIN) && (parep_mpi_initialized))) {
								real_fds[i].fd = -1;
								fds[i].revents = 0;
								pollret--;
								current_ipfd->num_active_fds--;
							}
						}
					}
					if(!has_bad_sock) has_bad_sock = 1;
				} else {
					current_ipfd->num_active_fds = nfds;
					for(int i = 0; i < nfds; i++) {
						if(fds[i].fd < 0) current_ipfd->num_active_fds--;
						real_fds[i].fd = fds[i].fd;
						real_fds[i].events = fds[i].events;
					}
				}
				pthread_mutex_unlock(&safe_to_pass_mutex);
				if(pollret != 0) return pollret;
				num_active_fds = current_ipfd->num_active_fds;
				pthread_mutex_lock(&safe_to_pass_mutex);
				if(num_active_fds > 0) {
					if((infinite_timeout) && (has_bad_sock)) {
						real_fds[nfds].fd = pollintrpipe[0];
						real_fds[nfds].events = POLLIN;
						pollret = 0;
						do {
							pthread_mutex_unlock(&safe_to_pass_mutex);
							pollret = real_poll(real_fds,nfds+1,timeout);
							pthread_mutex_lock(&safe_to_pass_mutex);
							for(int i = 0; i < nfds; i++) {
								if(real_fds[i].revents != 0) {
									fds[i].revents |= real_fds[i].revents;
								}
							}
							if(real_fds[nfds].revents != 0) {
								if(real_fds[nfds].revents & POLLIN) {
									int intr = 0;
									real_read(pollintrpipe[0],&intr,sizeof(int));
									assert(intr == 1);
									close(pollintrpipe[0]);
									close(pollintrpipe[1]);
									current_ipfd->num_active_fds = nfds;
									for(int i = 0; i < nfds; i++) {
										if(fds[i].fd < 0) current_ipfd->num_active_fds--;
										real_fds[i].fd = fds[i].fd;
										real_fds[i].events = fds[i].events;
									}
									pthread_mutex_unlock(&safe_to_pass_mutex);
									pollret = real_poll(real_fds,nfds,timeout);
									pthread_mutex_lock(&safe_to_pass_mutex);
									break;
								}
								pollret--;
							}
						} while(pollret == 0);
					}
					else {
						pthread_mutex_unlock(&safe_to_pass_mutex);
						pollret = real_poll(real_fds,nfds,timeout);
						pthread_mutex_lock(&safe_to_pass_mutex);
					}
					if(pollret <= 0) {
						pthread_mutex_unlock(&safe_to_pass_mutex);
						return pollret;
					}
					int bad_sock_detected = 0;
					for(int i = 0; i < nfds; i++) {
						if(real_fds[i].revents != 0) {
							fds[i].revents |= real_fds[i].revents;
							if((real_fds[i].revents & POLLHUP) || (real_fds[i].revents & POLLERR) || ((i >= nfds-parep_mpi_node_num) && (real_fds[i].revents & POLLIN) && (parep_mpi_initialized))) {
								bad_sock_detected = 1;
							}
						}
					}
					if(bad_sock_detected == 0) {
						pthread_mutex_unlock(&safe_to_pass_mutex);
						return pollret;
					}
					if(safe_to_pass) {
						pthread_mutex_unlock(&safe_to_pass_mutex);
						return pollret;
					}
				}
				pthread_mutex_unlock(&safe_to_pass_mutex);
				int cmd = CMD_MISC_PROC_FAILED;
				write(parep_mpi_coordinator_sock,&cmd,sizeof(int));
				write(parep_mpi_coordinator_sock,&infinite_timeout,sizeof(int));
				write(parep_mpi_coordinator_sock,&num_active_fds,sizeof(int));
			}
		} while(true);
	}
}

ssize_t read(int fd, void *buf, size_t count) {
	if(real_waitpid == NULL) {
		real_waitpid = dlsym(RTLD_NEXT,"waitpid");
		parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
	}
	if(real_poll == NULL) {
		real_poll = dlsym(RTLD_NEXT,"poll");
		parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
	}
	if(real_read == NULL) {
		real_read = dlsym(RTLD_NEXT,"read");
		parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
	}
	if(!getenv("PAREP_MPI_PROXY_HACKED") || strcmp(getenv("PAREP_MPI_PROXY_HACKED"),"1")) return real_read(fd,buf,count);
	
	int pre_errno = errno;
	ssize_t readret;
	readret = real_read(fd,buf,count);
	if((readret < 0) && ((errno == ENOENT) || (errno == ECONNRESET))) {
		while(1);
		if(safe_to_pass) {
			return readret;
		} else {
			errno = pre_errno;
			readret = 0;
			return readret;
		}
	}
	else return readret;
}
 
pid_t waitpid(pid_t pid, int *status, int options) {
	if(real_waitpid == NULL) {
		real_waitpid = dlsym(RTLD_NEXT,"waitpid");
	}
	if(real_poll == NULL) {
		real_poll = dlsym(RTLD_NEXT,"poll");
	}
	if(real_read == NULL) {
		real_read = dlsym(RTLD_NEXT,"read");
	}
	if(!getenv("PAREP_MPI_PROXY_HACKED") || strcmp(getenv("PAREP_MPI_PROXY_HACKED"),"1")) return real_waitpid(pid,status,options);
	pthread_mutex_lock(&serv_addr_mutex);
	if(!server_addr_set) {
		parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
		memset(&server_addr,0,sizeof(server_addr));
		server_addr.sun_family = AF_UNIX;
		strcpy(server_addr.sun_path,"#EMPIServermpiexec");
		server_addr.sun_path[0] = 0;
		
		check((parep_mpi_coordinator_sock = socket(AF_UNIX, SOCK_STREAM, 0)),"Failed to create socket mpiexec");
		int ret = connect(parep_mpi_coordinator_sock,(struct sockaddr *)(&server_addr),sizeof(server_addr));
		while(ret != 0) {
			ret = connect(parep_mpi_coordinator_sock,(struct sockaddr *)(&server_addr),sizeof(server_addr));
		}
		if(pipe(pollintrpipe) < 0) {
			perror("Pollintrpipe creation failed mpiexec\n");
			exit(1);
		}
		server_addr_set = true;
		
		pid_t mypid = getpid();
		write(parep_mpi_coordinator_sock,&mypid,sizeof(pid_t));
		
		pthread_create(&coordinator_poller,NULL,poll_coordinator,NULL);
	}
	pthread_mutex_unlock(&serv_addr_mutex);
	
	int pre_errno = errno;
	int wnohang_used = options & WNOHANG;
	pid_t res;
	if(wnohang_used) {
		res = real_waitpid(pid,status,options);
		if(res == 0) {
			return res;
		} else if(res == (pid_t)-1) {
			if(errno == ECHILD) {
				//Check server if its safe to return and then get info from list
				int stp;
				pthread_mutex_lock(&safe_to_pass_mutex);
				stp = safe_to_pass;
				pthread_mutex_unlock(&safe_to_pass_mutex);
				if(head == NULL) {
					return res;
				}
				if(stp) {
					pid_t ret = head->pid;
					*status = head->status;
					wpid_out *temp = head;
					head = head->next;
					if(head == NULL) {tail = NULL;}
					free(temp);
					errno = pre_errno;
					return ret;
				} else {
					*status = 0;
					pid_t ret = 0;
					errno = pre_errno;
					return ret;
				}
			}
		} else {
			wpid_out *wout = (wpid_out *)malloc(sizeof(wpid_out));
			wout->pid = res;
			wout->status = *status;
			wout->next = NULL;
			if(tail == NULL) {
				head = wout;
			} else {
				tail->next = wout;
			}
			tail = wout;
			//Check server if its safe to return and then get info from list
			int stp;
			pthread_mutex_lock(&safe_to_pass_mutex);
			stp = safe_to_pass;
			pthread_mutex_unlock(&safe_to_pass_mutex);
			if(stp == 0) stp = send_to_server(&res,1,wnohang_used);
			if(stp) {
				pid_t ret = head->pid;
				*status = head->status;
				wpid_out *temp = head;
				head = head->next;
				if(head == NULL) {tail = NULL;}
				free(temp);
				return ret;
			} else {
				*status = 0;
				pid_t ret = 0;
				return ret;
			}
		}
	} else {
		do {
			res = real_waitpid(pid,status,options);
			if(res == 0) {
				return res;
			} else if(res == (pid_t)-1) {
				if(errno == ECHILD) {
					//Wait until its safe to return and then get info from list
					int stp;
					pthread_mutex_lock(&safe_to_pass_mutex);
					while(safe_to_pass == 0) {
						pthread_cond_wait(&safe_to_pass_cond,&safe_to_pass_mutex);
					}
					stp = safe_to_pass;
					pthread_mutex_unlock(&safe_to_pass_mutex);
					assert(stp == 1);
					if(head == NULL) {
						return res;
					}
					if(stp) {
						pid_t ret = head->pid;
						*status = head->status;
						wpid_out *temp = head;
						head = head->next;
						if(head == NULL) {tail = NULL;}
						free(temp);
						errno = pre_errno;
						return ret;
					}
				}
			} else {
				wpid_out *wout = (wpid_out *)malloc(sizeof(wpid_out));
				wout->pid = res;
				wout->status = *status;
				wout->next = NULL;
				if(tail == NULL) {
					head = wout;
				} else {
					tail->next = wout;
				}
				tail = wout;
				//Check server if its safe to return and then get info from list if its safe
				int stp;
				pthread_mutex_lock(&safe_to_pass_mutex);
				stp = safe_to_pass;
				pthread_mutex_unlock(&safe_to_pass_mutex);
				if(stp == 0) stp = send_to_server(&res,1,wnohang_used);
				if(stp) {
					pid_t ret = head->pid;
					*status = head->status;
					wpid_out *temp = head;
					head = head->next;
					if(head == NULL) {tail = NULL;}
					free(temp);
					return ret;
				}
			}
		} while(res > 0);
	}
}
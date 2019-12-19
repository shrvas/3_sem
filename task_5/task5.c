#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

//global variables
//int size;
char *rec;
int j = 0;

void sig_switcher(int signo) 
{
	switch(signo)
    {
	case SIGUSR1:
		rec[j] = '0';
		j++;
	break;
	case SIGUSR2:
		rec[j] ='1';
		j++;
	break;
	default:
		break;
	}
}

int main() 
{
	int size;
	int i = 0;
	int fd;
	struct stat statbuf;
	if((fd = open("data.txt", O_RDONLY)) < 0)
    {
		printf("Can't open such file\n");
		return -1;
	}
	fstat(fd, &statbuf);
	size = statbuf.st_size;
	printf("Buffer size is %d bytes\n", size);
	
	char *data;
	data = malloc(size*sizeof(char));
	read(fd, data, size);
	close(fd);
	
	rec = malloc(size*sizeof(char));
	sem_t *sem;
	sem = sem_open("NAME1", O_CREAT, 0776, 0);
	
	struct timespec tim_sender, tim, tim_rec;
	int sleep_t = 10000000;
	tim_sender.tv_sec = 0;
	tim_sender.tv_nsec = sleep_t;
	tim_rec.tv_sec = 0;
	tim_rec.tv_nsec = sleep_t;
	srand(time(0));
	
	//forking process
	pid_t pid = fork();
	if(pid < 0)
    {
		printf("Forking failed\n");
		return -1;
	}
	printf("Pid is %d\n", pid);
	//sending from parent to child
	if(pid){
		sem_wait(sem);
		for(i = 0; i <size; i++){
			kill(pid, data[i] == '0' ? SIGUSR1 : SIGUSR2);
			nanosleep(&tim_sender, &tim);
		}
		sleep(1);
	}
	else{
		if(signal(SIGUSR1, sig_switcher) == SIG_ERR)
			printf("can't proceed with first signal\n");
		
		if(signal(SIGUSR2, sig_switcher) == SIG_ERR)
			printf("can't proceed with second signal\n");
		
		sem_post(sem);
		while(j < size)
			nanosleep(&tim_sender, &tim);
		for(i = 0; i < (size - 1); i++)
			printf("%c", rec[i]);
		
	}
	printf("\n");
	free(rec);
	free(data);
	return 0;
}
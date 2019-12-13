#define _GNU_SOURCE
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>

int transmission_type = 0;
int buffer_size = 1;
char *input_name = "data.mp4";
char *output_name = "output.mp4";

int main() {
    int pid = fork();
    
    char* buffer;
    int fifo;
    sem_t* sync[2];
    int shm_fd;
    mqd_t queue;
    char* buffer_shared;
    int* delta_shared;

    buffer = malloc(buffer_size + 1);
    switch(transmission_type) {
    case 0:;
        mkfifo("/tmp/fifo", 0666);
        if(pid)
            fifo = open("/tmp/fifo", O_WRONLY);
        else
            fifo = open("/tmp/fifo", O_RDONLY);
        break;
    case 1:;
        sync[0] = sem_open("/syncer1", O_CREAT, 0666, 0);
        sync[1] = sem_open("/syncer2", O_CREAT, 0666, 0);
        shm_fd = shm_open("/shared_mem", O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, buffer_size + 1 + sizeof(int));
        buffer_shared = mmap(NULL, buffer_size + 1 + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        delta_shared = (int*)&(buffer_shared[buffer_size + 1]);
        break;
    case 2:;
        struct mq_attr attr = {0, 10, buffer_size + 1, 0};
        queue = mq_open("/queue", O_CREAT | O_RDWR, 0666, &attr);
        break;
    default:
        break;
    }
    if(pid) {
        int input_fd = open(input_name, O_RDONLY);
        if(input_fd == -1) {
            printf("cant open\n");
            return 0;
        }
        int delta = 0;
        int last = 0;
        while(delta <= buffer_size) {
            delta = read(input_fd, buffer, buffer_size);
            if(delta == -1) {
                printf("cant read\n");
                return 0;
            }
            if(delta == 0) {
                delta = buffer_size + 1;
                printf("reached the end of file\n");
            }
            switch (transmission_type)
            {
            case 0:
                if(delta <= buffer_size)
                    write(fifo, buffer, delta);
                break;
            case 1:
                *delta_shared = delta;
                memcpy(buffer_shared, buffer, delta);
                sem_post(sync[0]);
                sem_wait(sync[1]);
                break;
            case 2:
                mq_send(queue, buffer, delta, 1);
                break;
            default:
                break;
            }
        }
    }
    else {
        int output_fd = open(output_name, O_WRONLY | O_CREAT, 0666);
        while(1) {
            int delta;
            switch (transmission_type)
            {
            case 0:
                delta = read(fifo, buffer, buffer_size);
                if(delta == 0)
                    delta = buffer_size + 1;
                break;
            case 1:
                sem_wait(sync[0]);
                delta = *delta_shared;
                memcpy(buffer, buffer_shared, delta);
                sem_post(sync[1]);
                break;
            case 2:
                delta = mq_receive(queue, buffer, buffer_size + 1, NULL);
                break;
            default:
                break;
            }
            if(delta == buffer_size + 1) {
                printf("file recieved\n");
                break;
            }
            write(output_fd, buffer, delta);            
        }
    }
    switch(transmission_type) {
    case 0:;
        close(fifo);
        break;
    case 1:;
        sem_close(sync[0]);
        sem_close(sync[1]);
        sem_unlink("/syncer1");
        sem_unlink("/syncer2");
        munmap(buffer_shared, buffer_size + 1 + sizeof(int));
        close(shm_fd);
        shm_unlink("/shared_mem");
        break;
    case 2:;
        mq_close(queue);
        mq_unlink("/queue");
        break;
    default:
        break;
    }
    if(pid)
        waitpid(pid, NULL, 0);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <resolv.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define MYDIR "/home/shrvas/Documents/3_sem_alexandra_sasha/task6/"

char newfiles[256] = "/home/shrvas/Documents/3_sem_alexandra_sasha/task6/";				

void Daemon(const char * dir);
void BackUp(const char * name);
void diff(const char *file, const char *name);
int iftext(const char *f);

int main(int argc, char * argv[]) {
	char dir[256] = MYDIR;  					
	pid_t pid;
	if (argc < 2) {
		printf("-d for daemon or -i for interactive\n");
		exit(1);
	}
	if (strcmp(argv[1], "-i") == 0) {
		Daemon(dir);
	}
	else if (strcmp(argv[1], "-d") == 0) {
		if ((pid = fork()) < 0) {
			perror("fork()");
			exit(1);
		}
		else if (pid) {
			exit(0);
		}
		pid_t sid = setsid();
		printf("sid: %d\n", sid);
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);

		Daemon(dir);
	}
	return 0;
}

void Daemon(const char * dir) {

	while (1) {
		BackUp(dir); 	 
		sleep(100);
	}
	return;
}

void BackUp(const char * name) {
    	DIR * dir = opendir(name);
    	if(dir) {
		char Path[PATH_MAX], *EndPtr = Path;
		char * endfile = newfiles + strlen(newfiles);
		struct stat info;
        	struct dirent * e;
        	strcpy(Path, name);
        	EndPtr += strlen(name);             
        	while ((e = readdir(dir)) != NULL) { 				//Iterates through the entire directory.	
			if (e->d_name[0] == '.') continue;
			strcpy(EndPtr, e -> d_name);
			if (!stat(Path, &info)) { 
				if (S_ISDIR(info.st_mode)) {
					strcat(Path, "/");
					strcpy(endfile, e -> d_name);
					strcat(newfiles, "/");
					char newdir[256] = "mkdir ";
					strcat(newdir, newfiles);
					system(newdir);
					BackUp(Path);
					strcpy(endfile, "");
				} else if (S_ISREG(info.st_mode)) { 
					if (iftext(Path)) 
						diff(Path, e->d_name);
				}
			}
		}  
	}
	closedir(dir);
	return; 
}

void diff(const char * file, const char * name) {
	char newfile[256];
	strcpy(newfile, newfiles);
	strcat(newfile, name);
	char arg[256] = "diff -q ";
	char files[256] = "";
        strcat(files, file);
	strcat(files, " ");
        strcat(files, newfile);
	strcat(arg, files);
	int fd2 = open(newfile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
        int fd = open("/home/shrvas/Documents/3_sem_alexandra_sasha/task6/backup/log.txt", O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
        if(fd < 0) { 
                exit(1);
        }
	dup2(fd, 1);
        system(arg);
	lseek(fd, 0, SEEK_SET); 
	char buf[1024];
	int x = read(fd, buf, 1024);
	if (x != 0) {
		char copy[256] = "cp ";
		strcat(copy, files);
		system(copy);
	}
	system("rm /home/shrvas/Documents/3_sem_alexandra_sasha/task6/backup/log.txt");
	close(fd);
	close(fd2);
	return;
}

int iftext(const char * file){
        char arg[256]= "file ";
        strcat(arg, file);
	int fd = open("/home/shrvas/Documents/3_sem_alexandra_sasha/task6/backup/log.txt", O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
	if(fd < 0) {
                exit(1);
        }
	dup2(fd, 1);
        system(arg);
	close(fd);
        fd = open("/home/shrvas/Documents/3_sem_alexandra_sasha/task6/backup/log.txt", O_RDONLY);
        struct stat statbuf;
        fstat(fd, &statbuf);
        int size = statbuf.st_size;

        char buf[256];
        int num = 200;
        num = read(fd, buf, size);
        if(num != size){
                printf("size = %d", size);
                printf("num = %d\n", num);
                printf("ERROR in read\n");
        }
        close(fd);
	system("rm /home/shrvas/Documents/3_sem_alexandra_sasha/task6/backup/log.txt");

        if(buf[size - 2] == 't' && buf[size-3] == 'x' && buf[size-4] == 'e' && buf[size-5] == 't') {
                return 1;
        }
	return 0;	
}
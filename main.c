#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>

#define BUFLEN 256
#define NARGS  256

void getargs(int *ac,char *av[],char *p);
void getargs_aug(char *p,char *lbuf2);
void sig_catch(int sig);

int main(void){
	char lbuf[BUFLEN];
	char lbuf2[2*BUFLEN];
	char *argv[NARGS];
	char *argv_aug[NARGS][NARGS];

	char *argv_a[10];
	char *argv_b[10];

	int flag=0;
	int flag2=0;
	int argc=0;
	int i=0;
	int j=0;
	int stat[20];
	int status;
	int exit_stat=0;
	int cnt;
	int pipboder[10]={0};
	int pipboder2[10]={0};
	int pfd[2];
	int pfd1[2];
	int pfd2[2];

	int fd;

	pid_t fork(void);
	pid_t getpid(void);
	pid_t getppid(void);

	pid_t pid;

	for(;;){
		signal(SIGINT,SIG_IGN);
		fprintf(stderr,"$:");
		if(fgets(lbuf,sizeof lbuf ,stdin)==NULL){
			putchar('\n');
			return 0;
		}
		lbuf[strlen(lbuf)-1]='\0';
		if(*lbuf=='\0')
			continue;
		getargs_aug(lbuf,lbuf2);
		getargs(&argc, argv, lbuf2);

		if(strcmp(argv[0],"cd")==0){
			chdir(argv[1]);
			continue;
		}

		argv[argc]=NULL;

		j=0;
		flag=0;
		flag2=0;
		pipboder[0]=0;
		pipboder2[0]=0;
		for(j=0;j<argc;j++){
			if(*argv[j]=='|'){//パイプ処理
				pipboder[flag]=j;
				argv[j]=NULL;
				flag++;
				j++;
				argv_aug[flag-1][j-pipboder[flag-1]]=NULL;
			}

			if(*argv[j]=='>'){//リダイレクト
				pipboder2[flag2]=j;
				argv[j]=NULL;
				flag2++;
				j++;
			}
			if(j>30){
				printf("return error\n");
				return 0;
			}


			if(flag==0 && flag2==0){
				argv_aug[0][j]=argv[j];
			}else if(flag >= 1){
				argv_aug[flag][j-pipboder[flag-1]-1]=argv[j];
			}
		}


		if(flag2>0){
			fd=open(argv[pipboder2[flag2-1]+1],O_WRONLY|O_CREAT|O_TRUNC,0644);//argv[pipboder2[flag2]]
			if(fork()==0){
				signal(SIGINT,SIG_DFL);
				close(1);
				dup(fd);
				close(fd);
				if(flag==0){
					execvp(argv[0],argv);
				}
				execvp(argv[pipboder[flag-1]+1],argv);
			}
			close(fd);
			wait(&status);
		}


		if(flag==0 && flag2==0){
			pid = fork();
			if(0 == pid){
				signal(SIGINT,SIG_DFL);
				execvp(argv[0],argv);
				wait(&status);
				exit(0);
			}else if(pid > 0){
				wait(&status);

				if(strcmp(argv[0],"exit")==0){
					return 0;
				}
			}else if(pid < 0){
				perror("fork");
				exit(EXIT_FAILURE);
			}
		}else if(flag >= 1 && flag2 ==0){
			pipe(pfd1);
			if(fork()==0){//子プロセス1
				signal(SIGINT,SIG_DFL);
				close(1);
				dup(pfd1[1]);
				close(pfd1[0]);close(pfd1[1]);
				execvp(argv_aug[0][0],argv_aug[0]);
			}

			if(flag>=2){
				pipe(pfd2);
				for(i=0;i<flag-1;i++){
					if(fork()==0){//子プロセス
						signal(SIGINT,SIG_DFL);
						if(i%2 ==0){
							close(0);
							dup(pfd1[0]);
							close(1);
							dup(pfd2[1]);//前のループのpfd
						}else if(i%2 !=0){
							close(0);
							dup(pfd2[0]);
							close(1);
							dup(pfd1[1]);
						}
						close(pfd1[0]);close(pfd1[1]);
						close(pfd2[0]);close(pfd2[1]);
						printf("%d,%s\n",i+1,argv_aug[i+1][0]);
						execvp(argv_aug[i+1][0],argv_aug[i+1]);

					}
					if(i%2 == 0){
						close(pfd1[0]);close(pfd1[1]);
						pipe(pfd1);
					}else if(i%2 != 0){
						close(pfd2[0]);close(pfd2[1]);
						pipe(pfd2);
					}
				}
			}

			if(fork()==0){//子プロセス2
				signal(SIGINT,SIG_DFL);
				if(flag>=2){
					if((i-1)%2 ==0){
						close(0);
						dup(pfd2[0]);
						close(pfd2[0]);close(pfd2[1]);
					}else if((i-1)%2 !=0){
						close(0);
						dup(pfd1[0]);
						close(pfd1[0]);close(pfd1[1]);
					}
				}else{
					close(0);
					dup(pfd1[0]);
					close(pfd1[0]);close(pfd1[1]);
				}

				execvp(argv_aug[flag][0],argv_aug[flag]);
			}
			close(pfd1[0]);close(pfd1[1]);
			if(flag>=2){close(pfd2[0]);close(pfd2[1]);}
			for(i=0;i<flag+1;i++){
				wait(&stat[i]);
			}
		}
	}
	return 0;
}


void getargs(int *ac,char *av[],char *p){
	*ac=0;
	for(;;){
		while(isblank(*p)){
			p++;
		}
		if(*p=='\0'){
			return;
		}
		av[(*ac)++]=p;//av[*(ac)++]=pにするとバグる
		while(*p && !isblank(*p))
			p++;

		if(*p=='\0'){
			return;
		}
		*p++ = '\0';
	}
}

void getargs_aug(char *p, char *lbuf2){
	for(;;){
		if(p==NULL) return;
		*lbuf2=*p;
		if(*p=='\0'){
			return;
		}
		if(*p== '|'){
			lbuf2++;
			*(lbuf2)=' ';
		}
		if(*(p+1) == '|'){
			lbuf2++;
			*lbuf2=' ';
		}
		if(*p== '>'){
			lbuf2++;
			*(lbuf2)=' ';
		}
		if(*(p+1) == '>'){
			lbuf2++;
			*lbuf2=' ';
		}
		if(*p== '<'){
			lbuf2++;
			*(lbuf2)=' ';
		}
		if(*(p+1) == '<'){
			lbuf2++;
			*lbuf2=' ';
		}
		*lbuf2++;
		p++;
	}
}


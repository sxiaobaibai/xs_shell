# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <stdlib.h>
#include <fcntl.h>

int main()
{
  char *buf = malloc(sizeof(char) * 10);
  char *src = "hello";
  strlcpy(buf, src, );
  printf("%lu, %s\n", strlen(src), buf);
  for (int i = 0; i < 10; ++i)
  {
    printf("%d, ", buf[i]);
  }
  printf("\n");
  //char *command1[] = {"ls", "-l", NULL};
  //char *command2[] = {"cat", NULL};
  //int status;
  //pid_t pid, pid2;
  //int pipe_fd[2];

  //int fd = open("test", O_RDONLY);
  ////int fd = open("test2", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  //pipe(pipe_fd);
  //pid = fork();
  //if(0 == pid){
  //  pid2 = fork();
  //  if (pid2 == 0)
  //  {
  //    close(pipe_fd[0]);
  //    dup2(pipe_fd[1], 1);
  //    close(pipe_fd[1]);
  //    signal(SIGINT,SIG_DFL);
  //    execvp(command1[0],command1);
  //    fprintf(stderr, "xs_shell: command not found: %s\n", command1[0]);
  //    exit(1);
  //  }
  //  else if (pid2 > 0)
  //  {
  //    signal(SIGINT,SIG_DFL);
  //    close(pipe_fd[1]);
  //    //close(0);
  //    dup2(pipe_fd[0], 0);
  //    dup2(fd, 0);
  //    //dup2(fd,1);
  //    //close(pipe_fd[0]);
  //    //dup2(fd, 0);
  //    execvp(command2[0],command2);
  //    fprintf(stderr, "xs_shell: command not found: %s\n", command2[0]);
  //    exit(1);
  //  }
  //}else if(pid > 0){
  //  close(pipe_fd[0]);
  //  close(pipe_fd[1]);
  //  wait(&status);
  //}else if(pid < 0){
  //  perror("fork");
  //  exit(EXIT_FAILURE);
  //}
  return 0;
}

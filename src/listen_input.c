#include "listen_input.h"
#include <ctype.h>
#include <fcntl.h>

#define COMMAND_BUF 256
#define MAX_DIR_LEN 128
#define MAX_FILENAME_LEN 128

int is_buildin(char *cmd)
{
  if (!cmd)
    return (0);

  if (strcmp(cmd, "cd") == 0)
    return (1);
  if (strcmp(cmd, "echo") == 0)
    return (1);
  if (strcmp(cmd, "exit") == 0)
    return (1);
  return (0);
}

int execute_buildin(char **cmd)
{
  size_t size;

  size = 0;
  while (cmd[size])
    size++;
  if (strcmp(*cmd, "cd") == 0)
  {
    //usage: "cd ~"  "cd ~/dir" "cd /home" "cd ../dir" "cd ./dir" "cd"
    if (size > 2)
      fprintf(stderr, "too many arguments for cd command.\n");
    if (size == 1)
      chdir(getenv("HOME"));
    else if (size == 2)
    {
      if (cmd[1][0] == '~')
      {
        char absolute_dir[MAX_DIR_LEN] = "";
        strncpy(absolute_dir, getenv("HOME"), MAX_DIR_LEN - 1);//leave one space for \0.
        if (strlen(absolute_dir) < MAX_DIR_LEN - 1)//leave one space for \0.
        {
          absolute_dir[strlen(absolute_dir)] = '/';
          absolute_dir[strlen(absolute_dir) + 1] = '\0';
          strlcat(absolute_dir, &cmd[1][1], MAX_DIR_LEN);
          chdir(absolute_dir);
        }
      }
      else
        chdir(cmd[1]);
    }
  }
  if (strcmp(*cmd, "exit") == 0)
  {
    exit(0);
  }
  return (0);
}


char *set_file_for_redirect(char *filename, char *src)
{
  int i;

  while (*src)
    if (isblank(*src))
      src++;
    else
      break ;
  i = 0;
  while (*src)
  {
    if (!isblank(*src) && *src != '<' && *src != '>')
      filename[i] = *src;
    else
      break ;
    src++;
    i++;
  }
  filename[i] = '\0';
  return (src);
}

static int set_redirect(int *fd, char *src)
{
  char filename[MAX_FILENAME_LEN];

  while (*src)
  {
    if (fd[1] == -1 && strncmp(src, ">>", 2) == 0)
    {
      src = set_file_for_redirect(filename, src + 2);
      if ((fd[1] = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
        return (-1);
    }
    if (fd[1] == -1 && strncmp(src, ">", 1) == 0)
    {
      src = set_file_for_redirect(filename, src + 1);
      if ((fd[1] = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        return (-1);
    }
    else if (*src == '<')
    {
      src = set_file_for_redirect(filename, src + 1);
      if ((fd[0] = open(filename, O_RDONLY)) < 0)
      {
        fprintf(stderr, "xs: no such file or directory: %s\n", filename);
        return (-1);
      }
    }
    else
    {
      src++;
    }
  }
  return (0);//if error -> return -1;
}

static int execute_command(char **command)
{
  int i;
  int redirect_fd[2];
  /*
  0: input fd(file descriptor),
  1: output fd
  */

  redirect_fd[0] = -1;
  redirect_fd[1] = -1;
  i = 0;
  while (command[i])
  {
    if (*command[i] == '>' || *command[i] == '<')
    {
      if (set_redirect(redirect_fd, command[i]) == -1)
      {
        command[i] = NULL;
        free(command[i + 1]);
        exit(-1);
      }
      command[i] = NULL;
      free(command[i + 1]);
      break ;
    }
    i++;
  }
  for (int j = 0; j < 2; ++j)
    if (redirect_fd[j] >= 0)
      dup2(redirect_fd[j], j);
  signal(SIGINT,SIG_DFL);
  execvp(command[0],command);

  fprintf(stderr, "xs: command not found: %s\n", command[0]);
  exit(1);
  return (0);
}

static int execute_non_piped_command(char **command)
{
  int status;
  pid_t pid;

  if (is_buildin(*command))
  {
    if (execute_buildin(command) != 0)
    {
      fprintf(stderr, "error with command: %s\n", *command);
      exit(1);
    }
  }
  else
  {
    pid = fork();
    if(0 == pid){

      execute_command(command);

      fprintf(stderr, "xs: command not found: %s\n", command[0]);
      exit(1);
    }
    else if(pid > 0)
    {
      wait(&status);
    }
    else if(pid < 0)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
  }
  return (0);
}

static const char *skip(char const *src, const char delim)
{
  while (*src)
  {
    if (*src != delim)
      break ;
    src++;
  }
  return (src);
}

static char **free_all(char **pp)
{
  char **head;

  head = pp;
  if (pp != NULL)
  {
    while (*head)
    {
      if (*head != NULL)
      {
        free(*head);
        *head = NULL;
      }
      head++;
    }
    free(pp);
  }
  return (NULL);
}

static char ***free_three(char ***pp)
{
  char ***head;

  head = pp;
  if (pp != NULL)
  {
    while (*head)
    {
      free_all(*head);
      *head = NULL;
      head++;
    }
    free(pp);
  }
  return (NULL);
}

static const char *next(char const *src, const char delim)
{
  while (*src)
  {
    if (*src == delim)
      break ;
    src++;
  }
  return (src);
}

static const char *next_delim_or_redirect(char const *src, const char delim)
{
  while (*src)
  {
    if (*src == delim || *src == '>' || *src == '<')
      break ;
    src++;
  }
  return (src);
}

static size_t calc_size(char const *src, const char delim)
{
  /*
  Caution: calculate how many elements is seperated by the delim character.
  Caution: if no delim exist, returned count is one.
  Caution: if no src string, count is zero.
  a|b||c -> return 4
  */
  size_t size;

  size = 1;
  if (src == NULL || *src == '\0')
    return (0);
  while (*src)
  {
    if (*src == delim)
    {
      size++;
    }
    src++;
  }
  return (size);
}

static size_t calc_sizeof_cmds_and_redirect(char const *src, const char delim)
{
  /*
  Caution: calculate how many elements is seperated by the delim character.
  Caution: if delim continus, skip all continous delims.
  a|b||c<a__ -> return 4 (a,b,c,<a__)
  */
  size_t size;

  size = 0;
  if (!*(src = skip(src, delim)))
    return (0);
  while (*src)
  {
    while (*src && *src != delim && *src != '>' && *src != '<')
      src++;
    size++;
    src = skip(src, delim);
    if (*src == '>' || *src == '<')
    {
      size++;
      break ;
    }
  }
  return (++size);
}

static char **split_by_delim(const char *src, char delim, size_t *num_items)
{
  /*
Caution: the returned char* vector will be NULL terminated
Caution: a||b will be counted as 3 commands -> 'a', '', 'b'
  */
  size_t size;
  const char *head;
  char **split;
  char **split_head;

  if (src == NULL)
    return (NULL);
  size = calc_size(src, delim);
  if ((split = (char **)malloc(sizeof(char *) * (size + 1))) == NULL)
    return (NULL);
  split_head = split;
  while (*src)
  {
    head = next(src, delim);
    if ((*split_head = malloc(sizeof(char) * (head - src + 1))) == NULL)
      return (free_all(split));
    strlcpy(*split_head, src, head - src + 1);
    split_head++;
    src = head + 1;
  }
  *split_head = NULL;
  *num_items = size;
  return (split);
}

static char **split_to_commands_and_redirect(const char *src, char delim, size_t *num_items)
{
  /*
  Caution: a|||||b will be counted as 2 commands -> 'a', 'b'
  */
  size_t size;
  const char *head;
  char **split;
  char **split_head;

  if (src == NULL)
    return (NULL);
  size = calc_sizeof_cmds_and_redirect(src, delim);
  if ((split = (char **)malloc(sizeof(char *) * (size + 1))) == NULL)
    return (NULL);
  split_head = split;
  src = skip(src, delim);
  while (*src)
  {
    head = next_delim_or_redirect(src, delim);
    if ((*split_head = malloc(sizeof(char) * (head - src + 1))) == NULL)
      return (free_all(split));
    strlcpy(*split_head, src, head - src + 1);
    split_head++;
    src = skip(head, delim);
    if (*src == '>' || *src == '<' )
    {
      size_t head_size = strlen(src) + 1;
      if ((*split_head = malloc(sizeof(char) * (strlen(src) + 1))) == NULL)
        return (free_all(split));
      strlcpy(*split_head, src, head_size);
      split_head++;
      break ;
    }
  }
  *split_head = NULL;
  *num_items = size;
  return (split);
}

static void recursive_multi_pipe(int step, char ***piped_argv)
{
  int pipe_fd[2];
  pid_t pid;
  int status;

  if (step == 0)
  {
    execute_command(piped_argv[step]);
    fprintf(stderr, "xs: command not found: %s\n", piped_argv[step][0]);
  }
  else
  {
    pipe(pipe_fd);
    pid = fork();
    if (pid == 0)
    {
      close(pipe_fd[0]);
      dup2(pipe_fd[1], 1);
      close(pipe_fd[1]);
      recursive_multi_pipe(step - 1, piped_argv);
    }
    else if (pid > 0)
    {
      close(pipe_fd[1]);
      dup2(pipe_fd[0], 0);
      close(pipe_fd[0]);

      wait(&status);
      execute_command(piped_argv[step]);
      fprintf(stderr, "xs: command not found: %s\n", piped_argv[step][0]);
      exit(1);
    }
    else if(pid < 0)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
  }
}

static int execute_piped_command(char ***commands, size_t num_pipe_items)
{
  int status;
  pid_t pid;

  int pipe_fd[2];
  pipe(pipe_fd);
  if ((pid = fork()) == 0)
  {
    close(pipe_fd[1]);
    dup2(pipe_fd[0], 0);
    close(pipe_fd[0]);
    recursive_multi_pipe(num_pipe_items - 1, commands);
  }
  else if (pid > 0)
  {
    close(pipe_fd[1]);
    close(pipe_fd[0]);
    wait(&status);
  }
  else if (pid < 0)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  return (0);
}

static char ***parse_into_piped_vectors(char *str, size_t *num_pipe_items)
{
  char **piped_items;
  char ***commands;
  size_t cnt_command;

  // pipe seperation
  // Caution: when input is as "||||||" -> output becomes wierd.
  piped_items = split_by_delim(str, '|', num_pipe_items);
  commands = malloc(sizeof(char***) * (*num_pipe_items + 1));
  for (size_t i = 0; i < *num_pipe_items; i++)
    commands[i] = split_to_commands_and_redirect(piped_items[i], ' ', &cnt_command);
  commands[*num_pipe_items] = NULL;
  free_all(piped_items);
  return (commands);
}

ssize_t listen_input(const char *prompt)
{
  char buf_input[COMMAND_BUF];
  char ***piped_items;
  size_t num_pipe_items;

  // ToDo tab complition
  fputs(prompt, stdout);
  fgets(buf_input, sizeof(buf_input), stdin);
  buf_input[strlen(buf_input) - 1] = '\0';

  piped_items = parse_into_piped_vectors(buf_input, &num_pipe_items);

  if (num_pipe_items == 1)
    execute_non_piped_command(piped_items[0]);
  else if (num_pipe_items > 1)
    execute_piped_command(piped_items, num_pipe_items);
  else
    return (-1);
  free_three(piped_items);
  return (0);
}

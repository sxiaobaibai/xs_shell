#include "listen_input.h"

#define COMMAND_BUF 256

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
    if (size > 2)
      fprintf(stderr, "too many arguments for cd command.\n");
    if (size == 1)
      chdir(getenv("HOME"));
    else if (size == 2)
      chdir(cmd[1]);
  }
  if (strcmp(*cmd, "exit") == 0)
  {
    exit(0);
  }
  return (0);
}

static int execute_command(char **command)
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
      signal(SIGINT,SIG_DFL);
      execvp(command[0],command);
      fprintf(stderr, "xs_shell: command not found: %s\n", command[0]);
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

static const char	*skip(char const *src, const char delim)
{
	while (*src)
	{
		if (*src != delim)
			break ;
		src++;
	}
	return (src);
}

static char			**free_all(char **pp)
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

static char			***free_three(char ***pp)
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

static const char	*next(char const *src, const char delim)
{
	while (*src)
	{
		if (*src == delim)
			break ;
		src++;
	}
	return (src);
}

static size_t		calc_size(char const *src, const char delim)
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

static size_t		calc_non_zero_item_size(char const *src, const char delim)
{
  /*
  Caution: calculate how many elements is seperated by the delim character.
  Caution: if delim continus, skip all continous delims.
  a|b||c -> return 3
  */
	size_t size;

	size = 0;
	if (!*(src = skip(src, delim)))
		return (0);
	while (*src)
	{
		if (*src++ == delim)
		{
			src++;
			size++;
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
	size_t		size;
	const char	*head;
	char		**split;
	char		**split_head;

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

static char **split_by_continuos_delim(const char *src, char delim, size_t *num_items)
{
  /*
  Caution: a|||||b will be counted as 2 commands -> 'a', 'b'
  */
  size_t size;
	const char	*head;
	char		**split;
	char		**split_head;

	if (src == NULL)
		return (NULL);
	size = calc_non_zero_item_size(src, delim);
	if ((split = (char **)malloc(sizeof(char *) * (size + 1))) == NULL)
		return (NULL);
	split_head = split;
	src = skip(src, delim);
	while (*src)
	{
		head = next(src, delim);
		if ((*split_head = malloc(sizeof(char) * (head - src + 1))) == NULL)
			return (free_all(split));
		strlcpy(*split_head, src, head - src + 1);
		split_head++;
		src = skip(head, delim);
	}
	*split_head = NULL;
  *num_items = size;
	return (split);
}

static void recursive_multi_pipe(int step, char ***piped_argv)
{
  int pipe_fd[2];
  pid_t pid;

  if (step == 0)
  {
    signal(SIGINT,SIG_DFL);
    execvp(piped_argv[step][0], piped_argv[step]);
    fprintf(stderr, "xs_shell: command not found: %s\n", piped_argv[step][0]);
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
      signal(SIGINT,SIG_DFL);
      execvp(piped_argv[step][0], piped_argv[step]);
      fprintf(stderr, "xs_shell: command not found: %s\n", piped_argv[step][0]);
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
    for (size_t i = 0; i < num_pipe_items - 1; i++) {
      wait(&status);
    }
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
    commands[i] = split_by_continuos_delim(piped_items[i], ' ', &cnt_command);
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
    execute_command(piped_items[0]);
  else if (num_pipe_items > 1)
    execute_piped_command(piped_items, num_pipe_items);
  else
    return (-1);
  free_three(piped_items);
  return (0);
}

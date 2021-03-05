#include "listen_input.h"

#define COMMAND_BUF 256
#define NUMPIPEARGS 256

//char *get_str_until_delim(char delim, char **buf)
//{
//  int len;
//  char *ret;
//
//  len = 0;
//  while ((*buf)[len] && (*buf)[len] != delim)
//    len++;
//  ret = malloc(sizeof(char) * (len + 1));
//  strncpy(ret, *buf, len + 1);
//  ret[len] = '\0';
//  *buf += len;
//  return ret;
//}
//
//int is_redirect(char *buf)
//{
//  if (*buf == '|' || *buf == '>' ||  *buf == '<')
//  {
//    return (1);
//  }
//  return (0);
//}
//
//char *skip_blank(char *str)
//{
//  while (*str && *str == ' ')
//  {
//    str++;
//  }
//  return str;
//}

int execute_command_in_new_process(char **command)
{
  int status;
  pid_t pid;

  pid = fork();
  if(0 == pid){
    signal(SIGINT,SIG_DFL);
    execvp(command[0],command);
    wait(&status);
    exit(0);
  }else if(pid > 0){
    wait(&status);
  }else if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  return (0);
}

int is_c_in_str(char c, const char *str)
{
  while (*str)
  {
    if (c == *str)
      return (1);
    str++;
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

char **split_by_delim(const char *src, char delim, size_t *num_items)
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

char **split_by_continuos_delim(const char *src, char delim, size_t *num_items)
{
  /*
  Caution: a||b will be counted as 2 commands -> 'a', 'b'
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

ssize_t listen_input(const char *prompt)
{
  // if error return -1;
  char buf_input[COMMAND_BUF];
  //char *piped_argv[NUMPIPEARGS];
  char **piped_argv;
  size_t num_pipe_items;

  fputs(prompt, stdout);

  // get a line (batch)
  // ToDo tab complition
  fgets(buf_input, sizeof(buf_input), stdin);
  buf_input[strlen(buf_input) - 1] = '\0';

  // pipe seperation
  // Caution: |||||| -> output is wierd.
  piped_argv = split_by_delim(buf_input, '|', &num_pipe_items);
  int i = 0;
  while (piped_argv[i])
  {
    printf("pipe: %d / %zu, %s\n", i, num_pipe_items, piped_argv[i]);
    i++;
  }
  // execute command
  if (num_pipe_items == 1)
  {
    char **command;
    size_t cnt_command;
    // ToDo: delim should be based on isBlank.
    command = split_by_continuos_delim(piped_argv[0], ' ', &cnt_command);
    execute_command_in_new_process(command);
    free_all(command);
  }
  else if (num_pipe_items > 1)
  {

  }
  else
  {
    return (-1);
  }

  // post process for memory allocation
  free_all(piped_argv);
  return (0);
}

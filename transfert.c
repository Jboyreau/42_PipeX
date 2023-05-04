/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   transefert.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jboyreau <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 14:06:58 by jboyreau          #+#    #+#             */
/*   Updated: 2023/05/04 15:36:58 by jboyreau         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "pipex.h"
#define STDIN 0

static char	fill_pipe(int *fd2, char *str)
{
	char	*buffer;
	int		inc;

	while (1)
	{
		buffer = gnl(STDIN);
		if (buffer == NULL)
			return (-1);
		inc = 0;
		while (*(buffer + inc))
			++inc;
		write(*(fd2 + 1), buffer, inc);
		inc = -1;
		while (*(str + (++inc)))
			if (*(buffer + inc) != *(str + inc))
				break ;
		if (*(buffer + inc) == '\n' && *(str + inc) == '\0')
		{
			free(buffer);
			return (0);
		}
		free(buffer);
	}
	return (-1);
}

static char	define_in_out2(int *fd2, t_mi m, int *old_read, char **argv)
{
	if (m.i == 3 && m.mode == 2)
	{
		if (pipe(fd2) == -1)
			return (-1);
		if (fill_pipe(fd2, *(argv + 2)) == -1)
			return (-1);
		*old_read = *fd2;
	}
	return (close(*(fd2 + 1)), 0);
}

static char	define_in_out(t_fds f, char **argv, int old_read, t_mi m)
{
	if (pipe(f.fd) == -1)
		return (-1);
	if ((m.i == 2 && m.mode == 1))
	{
		if (dup2(*(f.fd + 2), 0) == -1)
			return (-1);
		if (dup2(*(f.fd + 1), 1) == -1)
			return (-1);
		return (0);
	}
	if (m.i == (*(f.fd + 4)))
	{
		if (dup2(old_read, 0) == -1)
			return (-1);
		if (dup2(*(f.fd + 3), 1) == -1)
			return (-1);
		return (close(old_read), 0);
	}
	if (define_in_out2(f.fd2, m, &old_read, argv) == -1)
		return (-1);
	if (dup2(old_read, 0) == -1)
		return (-1);
	if (dup2(*(f.fd + 1), 1) == -1)
		return (-1);
	return (close(old_read), 0);
}

pid_t	transfer(char **arg, t_tabs t, int *fd, t_mi mi)
{
	pid_t		fout;
	static int	old_read;
	int			fd2[2];
	t_fds		f;

	f.fd = fd;
	f.fd2 = fd2;
	if (define_in_out(f, t.argv, old_read, mi) == -1)
		return (-1);
	fout = fork();
	if (fout == -1)
		return (-1);
	if (fout > 0)
	{
		old_read = *fd;
		close(*(fd + 1));
	}
	else
		if (execve(*arg, arg, t.env) == -1)
			return (-1);
	return (fout);
}

char	open_files(int *fd, int argc, char **argv, char mode)
{
	if (mode == 1)
	{
		*(fd + 2) = open(*(argv + 1), O_RDONLY);
		if (*(fd + 2) == -1)
			return (-1);
		*(fd + 3) = open(*(argv + argc - 1), O_WRONLY | O_TRUNC);
		if (*(fd + 3) == -1)
			return (close(*(fd + 2)), -1);
		*(fd + 4) = argc - 2;
		return (0);
	}
	*(fd + 3) = open(*(argv + argc - 1), O_WRONLY | O_APPEND);
	if (*(fd + 3) == -1)
		return (close(*(fd + 2)), -1);
	*(fd + 4) = argc - 2;
	return (0);
}

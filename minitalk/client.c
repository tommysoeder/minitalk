#include "minitalk.h"

static void	send_char(pid_t pid, unsigned char c)
{
	int	i;

	i = 7;
	while (i >= 0)
	{
		if ((c >> i) & 1)
		{
			if (kill(pid, SIGUSR2) == -1)
			{
				ft_putstr_fd("Error: kill(SIGUSR2)\n", 2);
				exit(1);
			}
		}
		else
		{
			if (kill(pid, SIGUSR1) == -1)
			{
				ft_putstr_fd("Error: kill(SIGUSR1)\n", 2);
				exit(1);
			}
		}
		usleep(300);
		i--;
	}
}

int	main(int argc, char **argv)
{
	pid_t	pid;
	char	*msg;
	size_t	i;

	if (argc != 3)
	{
		ft_putstr_fd("Uso: ./client <PID> <mensaje>\n", 2);
		return (1);
	}
	pid = (pid_t)ft_atoi(argv[1]);
	if (pid <= 0)
	{
		ft_putstr_fd("Error: PID invalido\n", 2);
		return (1);
	}
	msg = argv[2];
	i = 0;
	while (1)
	{
		send_char(pid, (unsigned char)msg[i]);
		if (msg[i] == '\0')
			break ;
		i++;
	}
	return (0);
}

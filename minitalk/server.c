#include "minitalk.h"

static void	signal_handler(int sig)
{
	static int				bit = 0;
	static unsigned char	c = 0;

	if (sig == SIGUSR2)
		c |= (1 << (7 - bit));
	bit++;
	if (bit == 8)
	{
		write(1, &c, 1);
		if (c == '\0')
			write(1, "\n", 1);
		bit = 0;
		c = 0;
	}
}

int	main(void)
{
	struct sigaction	sa;
	pid_t				pid;

	pid = getpid();
	ft_putstr_fd("Server PID: ", 1);
	ft_putnbr_fd(pid, 1);
	ft_putstr_fd("\n", 1);
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = &signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1
		|| sigaction(SIGUSR2, &sa, NULL) == -1)
	{
		ft_putstr_fd("Error: sigaction\n", 2);
		return (1);
	}
	while (1)
		pause();
	return (0);
}

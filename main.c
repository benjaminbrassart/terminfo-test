/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbrassar <bbrassar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 16:54:33 by bbrassar          #+#    #+#             */
/*   Updated: 2023/11/14 20:22:34 by bbrassar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curses.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>

// struct history_node {
// 	char *line;
// 	struct history_node *next;
// };

// static char *line = NULL;
static char line[256];
// static size_t line_capacity = 0;
static size_t line_length = 0;
static int term_lines;
static int term_columns;
// static struct history_node *history_start = NULL;
// static struct history_node *history_pos = NULL;

static int _putchar(int c)
{
	return write(STDOUT_FILENO, &c, 1);
}

static void _get_term_dimensions(void)
{
	struct winsize wz;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &wz);

	term_lines = wz.ws_row;
	term_columns = wz.ws_col;

	printf("Terminal size: %d by %d\n", term_columns, term_lines);
}

static void _setup_terminal(void)
{
	struct termios term;

	if (tgetent(NULL, getenv("TERM")) != 1)
		return;

	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
	_get_term_dimensions();
}

static void _clear_line(void)
{
	line_length = 0;
	line[0] = '\0';
}

static void _handle_sigwinch()
{
	signal(SIGWINCH, SIG_IGN);
	_get_term_dimensions();
	signal(SIGWINCH, _handle_sigwinch);
}

static void _handle_sigint()
{
	signal(SIGINT, SIG_IGN);
	_clear_line();
	signal(SIGINT, _handle_sigint);
}

static void _handle_sigquit()
{
}

static void _setup_signal(void)
{
	signal(SIGWINCH, _handle_sigwinch);
	signal(SIGINT, _handle_sigint);
	signal(SIGQUIT, _handle_sigquit);
}

static void _clear_terminal(void)
{
	tputs(tgetstr("cl", NULL), 1, _putchar);
}

static void _display_prompt(char const *prompt)
{
	if (prompt != NULL)
		write(STDERR_FILENO, prompt, strlen(prompt));
}

#define ESC_INSERT "\033[2~"
#define ESC_DELETE "\033[3~"
#define ESC_INSERT "\033[4~"
#define ESC_PGUP "\033[5~"
#define ESC_PGDOWN "\033[6~"
#define ESC_UP "\033[A"
#define ESC_DOWN "\033[B"
#define ESC_RIGHT "\033[C"
#define ESC_LEFT "\033[D"
#define ESC_END "\033[F"
#define ESC_HOME "\033[H"

static void _control_char(char const *seq, int len)
{
	printf("0x%02x 0x%02x 0x%02x 0x%02x\n", seq[0], seq[1], seq[2], seq[3]);

	if (seq[1] == '[') {
		switch (seq[2]) {
		case 'A': // up
			break;
		case 'B': // down
			break;
		case 'C': // right
			break;
		case 'D': // left
			break;
		case 'F': // end
			printf("end!\n");
			break;
		case 'H': // end
			printf("home!\n");
			break;
		default:
			break;
		}
	}

	// printf("seqnum = %08x\n", seqnum);

	(void)len;
}

static char *ft_readline(char const *prompt)
{
	char seq[4];
	int rr;

	_clear_line();
	_display_prompt(prompt);

	for (;;) {
		memset(seq, 0, sizeof(seq));

		if ((rr = read(STDIN_FILENO, &seq, sizeof(seq))) <= 0)
			return NULL;

		// printf("0x%02x\n", seq);

		switch (seq[0]) {
		case 0x04: // EOT (ctrl+d)
			if (line_length == 0)
				return NULL;
			break;
		case 0x0a: // LF (\n)
			_putchar('\n');
			if (line_length == 0) {
				_display_prompt(prompt);
				break;
			}
			return line;
		case 0x0c: // FF (ctrl+l)
			_clear_terminal();
			_display_prompt(prompt);
			break;
		case 0x1b:
			if (rr > 1)
				_control_char(seq, rr);
			break;
		case 0x7f: // DEL
			if (line_length > 0) {
				line[line_length] = '\0';
				line_length -= 1;
				write(STDOUT_FILENO, "\x7f", 1);
			}
			break;
		default:
			line[line_length] = seq[0];
			line_length += 1;
			line[line_length] = '\0';
			write(STDOUT_FILENO, seq, 1);
			break;
		}
	}
}

int main(void)
{
	_setup_terminal();
	_setup_signal();

	char *line;

	while ((line = ft_readline("> ")) != NULL) {
		printf("%s\n", line);
	}
}

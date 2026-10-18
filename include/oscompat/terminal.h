#ifndef OSCOMPAT_TERMINAL_H
#define OSCOMPAT_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Return 1 when fd refers to a terminal, otherwise 0. */
int os_isatty(int fd);

/* Write the terminal column and row counts through cols and rows; return 0 on success, -1 on failure. */
int os_tty_size(int fd, int* cols, int* rows);

/* Return 1 when fd is a terminal that accepts color output, otherwise 0. */
int os_color_enabled(int fd);

#ifdef __cplusplus
}
#endif

#endif

#ifndef SYNTHAX_SHELL_H
#define SYNTHAX_SHELL_H

void shell_init(void);
void shell_prompt(void);
void shell_execute_command(char *input);
void shell_run(void) __attribute__((noreturn));

#endif

#ifndef KERNEL_COM_H
#define KERNEL_COM_H

void com_initialize(void);
void com_write_string(char *s);
char com_getc();

void com_putc(char c);

#endif
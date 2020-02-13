#ifndef UART_SERVER_H
#define UART_SERVER_H

#define INVALID_UART_SERVER -1

int Getc(int tid, int channel);
int Putc(int tid, int channel, char ch);

void com1_getc_notifer(void);
void com1_putc_worker(void);

void com2_getc_notifer(void);
void com2_getc_worker(void);

void uart_server(void);

#endif
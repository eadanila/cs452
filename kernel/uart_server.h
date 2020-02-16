#ifndef UART_SERVER_H
#define UART_SERVER_H

#define INVALID_UART_SERVER -1
#define OTHER_TASK_QUEUED -2

#define SINGLE_SENSOR_DUMP_OFFSET 192
#define MULTI_SENSOR_DUMP_OFFSET 128

int Getc(int tid, int channel);
int Putc(int tid, int channel, char ch);

void uart1_getc_notifer(void);
void uart1_putc_notifer(void);

void uart2_getc_notifer(void);
void uart2_putc_notifer(void);

void uart1_server(void);
void uart2_server(void);

void create_uart_servers(void); // Create a priority 0

#endif
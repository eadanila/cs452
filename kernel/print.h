#ifndef PRINT_H
#define PRINT_H

void Print( int tid, char *fmt, ... );
void UPrint(int tid, char* str);

void MoveCursor(int tid, int x, int y);
void ClearScreen(int tid);

#endif
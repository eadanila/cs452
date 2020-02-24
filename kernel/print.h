#ifndef PRINT_H
#define PRINT_H

// Print functions that use a uart server's Putc function internally

// Prints a string with formatting
void Print( int tid, char *fmt, ... );
// Prints a string as is, does not treat %'s as formatting characters
void UPrint(int tid, char* str);

void MoveCursor(int tid, int x, int y);
void ClearScreen(int tid);

#endif
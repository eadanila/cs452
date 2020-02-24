#include "print.h"
#include <ts7200.h>
#include <bwio.h>
#include "uart_server.h"

// These functions mirror those used in bwio.c, but use Putc interally instead.
// Some functions which do not print characters were copied over to avoid 
// changing the bwio static library.

// Copy and rename of bwa2d
int a2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

// Copy and rename of bwa2i
char a2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = a2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

// Copy and rename of bwui2d
void ui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

// Copy and rename of bwi2a
void i2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	ui2a( num, 10, bf );
}

void putw( int tid, int channel, int n, char fc, char *bf ) 
{
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) Putc(tid, channel, fc );
	while( ( ch = *bf++ ) ) Putc(tid,  channel, ch );
}

void format ( int tid, char *fmt, va_list va ) 
{
	char bf[12];
	char ch, lz;
	int w;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			Putc( tid, COM2, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				Putc(tid, COM2, va_arg( va, char ) );
				break;
			case 's':
				putw( tid, COM2, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				putw( tid, COM2, w, lz, bf );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				putw( tid, COM2, w, lz, bf );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				putw( tid, COM2, w, lz, bf );
				break;
			case '%':
				Putc( tid, COM2, ch );
				break;
			}
		}
	}
}

void Print( int tid, char *fmt, ... )
{
    va_list va;

	va_start(va,fmt);
	format( tid, fmt, va );
	va_end(va);
}

void UPrint(int tid, char* str)
{
	for(;*str;str++) Putc(tid, COM2, *str);
}

void ClearScreen(int tid)
{
	Print(tid, "\033[2J");
}

void MoveCursor(int tid, int x, int y)
{
	Print(tid, "\033[%d;%dH", y, x);
}
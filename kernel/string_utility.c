#include "string_utility.h"
#include "logging.h"

void _strcpy(char* dest, const char* src)
{
	while(*src)
	{
		*dest = *src;
		src++;
		dest++;
	}
	*dest = 0;
}

int _strlen(const char* s)
{
	int i = 0;
	while(*s){ i++; s++; }
	return i;
}

int _strcmp(const char* str1, const char*str2)
{
	int r = 0;
	while(*str1 && *str2 && *str1 == *str2){ r++; str1++; str2++; }

	r = r + 1;

	if(*str1 == 0 && *str2 == 0) return 0;
	else if (*str1 == 0) return -r;
	else if (*str2 == 0) return r;
	else if (*str1 < *str2) return -r;
	else return r;
}

int _strsim(const char* str1, const char*str2)
{
	int r = 0;
	while(*str1 && *str2 && *str1 == *str2){ r++; str1++; str2++; }
	return r;
}

void itos(int i, char* s)
{	
	if (i == 0)
	{
		s[0] = '0';
		s[1] = 0;
		return;
	}

	int len = 0;
	int curr = i;
	for(;curr > 0; curr /= 10) len++;

	int is_negative = i < 0; // Offset to store '-'
	s[len + is_negative] = 0;
	char* null_terminator = &s[len + is_negative];
	s += (len + is_negative) - 1;

	curr = i;
	while(len > 0)
	{
		assert(s != null_terminator);
		*s = '0' + curr % 10;
		s--;
		curr /= 10;
		len--;
	}

    // If i is negative, the above loop will have 
	// stopped 1 element before the first.
	assert(s - 1 != null_terminator);
    if(i < 0) *(s-1) = '-';

	// Suppress pedantic
	null_terminator++;
}

int stoi(char* s)
{
	// TODO Error check?
	int r = 0;
    int negative = (*s == '-');

    if(negative) s++;

	while(*s && *s <= '9' && *s >= '0')
	{
		r *= 10;
		r += *s - '0';
		s++;
	}

    if(negative) return -r;
	return r;
}

// Takes a pointer to a char buffer that will be incremented
// to the next available spot after placing a character. This
// pointer is set to 0 if room in the buffer has run out.
// "size" represents the space left in the buffer, and is decremented 
// if the character is placed in the buffer.
void sputc(char** result, int* size, char c)
{
	if(*size <= 0 || (*result) == 0)
	{
		(*result) = 0;
		return;
	} 

	**result = c;
	(*result)++;
	(*size)--;
}

// Analagous to bwputw but uses sputc instead of bwputc to
// "print" to a buffer instead of directly to terminal.
void sputw( char** result, int* size, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) sputc( result, size, fc );
	while( ( ch = *bf++ ) ) sputc( result, size, ch );
}

// The following functions which do not print characters were copied from bwio.c to avoid 
// changing the bwio static library.

int sa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char sa2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = sa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void sui2a( unsigned int num, unsigned int base, char *bf ) {
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

void si2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	sui2a( num, 10, bf );
}

// Functions the same as bwformat except "prints" to the buffer "result"
// instead of directly to terminal.  "size" represents the size of the
// buffer. Returns non-zero if space in the buffer ran out, in which case
// the "printed" string has been truncated.
int _format_string ( char* result, int size, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			sputc( &result, &size, ch );
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
				ch = sa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: 
				if(result != 0) *result = 0;
				return result == 0;
			case 'c':
				sputc( &result, &size, va_arg( va, char ) );
				break;
			case 's':
				sputw( &result, &size, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				sui2a( va_arg( va, unsigned int ), 10, bf );
				sputw(  &result, &size, w, lz, bf );
				break;
			case 'd':
				si2a( va_arg( va, int ), bf );
				sputw( &result, &size, w, lz, bf );
				break;
			case 'x':
				sui2a( va_arg( va, unsigned int ), 16, bf );
				sputw( &result, &size, w, lz, bf );
				break;
			case '%':
				sputc( &result, &size, ch );
				break;
			}
		}
	}
	
	// Return an error if the result buffer provided was not large enough
	if(result != 0) *result = 0;
	return result == 0;
}

int format_string ( char* result, int size, char *fmt, ... )
{
	int r = 0;
	va_list va;

	va_start(va,fmt);
	r = _format_string(result, size, fmt, va);
	va_end(va);

	return r;
}

// Store an int into a 4 byte long char array
void pack_int(int i, char* buffer)
{
    // int* b = (int*) buffer;
    // *b = i;
    
    unsigned char* b = (unsigned char*) buffer;
    for(int j = 3; j >= 0; j--)
    {
        b[j] = i & 0xff;
        i = i >> 8;
    }
}

// Extract an int from a 4 byte long char array create by pack_int
int unpack_int(char* buffer)
{
    // int* b = (int*) buffer;
    // return *b;

    unsigned char* b = (unsigned char*) buffer;
    int r = 0;
    for(int i = 0; i != 3; i++)
    {
        r |= (unsigned int)b[i];
        r = r << 8;
    }
    r |= (unsigned int)b[3];

    return r;
}
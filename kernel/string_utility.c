#include "string_utility.h"

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
	s += (len + is_negative) - 1;

	curr = i;
	while(len > 0)
	{
		*s = '0' + curr % 10;
		s--;
		curr /= 10;
		len--;
	}

    // If i is negative, the above loop will have 
	// stopped 1 element before the first.
    if(i < 0) *(s-1) = '-';
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

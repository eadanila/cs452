#include <bwio.h>
#include <ts7200.h>
#include <limits.h>

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

	s[len] = 0;
	s += len - 1;

	curr = i;
	while(len > 0)
	{
		*s = '0' + curr % 10;
		s--;
		curr /= 10;
		len--;
	}
}

void printi(int i)
{
	char str[32];
	itos(i, str);
	bwputstr(COM2, str);
}

extern int arg_return_test(int i);

void test_1()
{
    int * temp;
    temp = 0x0100000;
    *temp = 0;

    int r = arg_return_test(24);

    printi(*temp);
    bwputstr(COM2, "\r\n");
    printi(r);

    bwputstr(COM2, "reached");
}

// make && cp test.elf /u/cs452/tftp/ARM/vscurtu/
int main( int argc, char* argv[] ) { 
    bwsetspeed( COM2, 115200 );
	bwsetfifo( COM2, OFF ); 

    bwputstr(COM2, "test\r\n");  

    test_1();

    return 0;
}
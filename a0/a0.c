 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>
#include <limits.h>

// #include <stdio.h>
// #include <string.h>

struct train_command
{
	int command_id;
	int arg1;
	int arg2;
};

#define BUFFER_SIZE 10000 
#define FIFO_ON 1

struct byte_buffer
{
	char data[BUFFER_SIZE];
	int size;
	int start;
	int end;
};


struct byte_buffer init_buffer(struct byte_buffer* r)
{
	r->size = 0;
	r->start = 0;
	r->end = 0;
}

void add_element(struct byte_buffer *b, char c)
{
	b->data[b->end] = c;
	if(b->size != BUFFER_SIZE) b->size++;

	b->end = (b->end + 1)%BUFFER_SIZE;
}

// Always check size first
char remove_element(struct byte_buffer *b)
{
	char r = b->data[b->start]; 
	b->start = (b->start + 1)%BUFFER_SIZE;
	b->size--;
	return r;
}

struct train_command create_train_command(int id, int arg1, int arg2)
{
	struct train_command r;
	r.command_id = id;
	r.arg1 = arg1;
	r.arg2 =arg2;
	return r;
}

// Globals
// MUST BE INITITALIZED IN A FUNCTION
struct byte_buffer char_buffer_com1;
struct byte_buffer char_buffer_com2;
int char_buffer_com1_size;
int char_buffer_com2_size;
int char_buffer_com1_front;
int char_buffer_com2_front;

struct byte_buffer read_buffer_com1;
struct byte_buffer read_buffer_com2;
int read_buffer_com1_size;
int read_buffer_com2_size;
int read_buffer_com1_front;
int read_buffer_com2_front;
char time_str[32];
char command[64];
int MAX_COMMAND_LEN;
int command_len;
int last_command_len;
int COMMAND_PRINT_HEIGHT;
int initializing;

int train_speeds[80]; // Train speed as of last command pushed
int* trains;
int trains_cnt;
int* switches;
char switch_states[256];
int switches_cnt;

int last_time;
int last_print_time;
int last_sensor_time;
int TIMER_SPEED;

// TODO Create circular buffer struct
struct train_command train_commands[BUFFER_SIZE];
int train_times[BUFFER_SIZE];
int train_buffer_size;
int train_buffer_start;
int train_buffer_end;
int last_train_command_time;
int current_wait_time;

int exit;
int initialize_track;

int sensor_states[16];
int sensor_states_len;
int sensor_byte;
char sensor_bytes[10];
int dump_finished;
int sensor_updated;
int all_sensor_states[128];

int switch_updated;
int marklin_dumping;

int curr_time;
int ran;

int last_sensor_request_time;
int last_sensor_first_byte;
int last_sensor_last_byte;
int bytes_read;

void init_globals();
void strcpy(char* dest, char* src);
int strlen(char* s);
int strcmp(char* str1, char*str2);
void itobs(int i, char* s);
void itos(int i, char* s);
int put_char( int channel, char c );
int putc_buffer(int channel);
int get_char( int channel, char* c);
int getc_buffer( int channel);
int putstr( int channel, char *str );
void putw( int channel, int n, char fc, char *bf );
void format ( int channel, char *fmt, va_list va );
void printf( int channel, char *fmt, ... );
char naivegetc(int channel);
void cursor_upper_left();
void clear_screen();
void init_clock();
int get_time();
float tics_to_seconds(int tics);
int seconds_to_tics(float seconds);
void command_append(char c);
void command_popback();
void command_clear();
void printi(int i);
void add_train_command(struct train_command command_id, int time);
void process_train_command(int curr_time);
int stoi(char* s);
void switch_track(int s_number, char dir);
void process_command();
void process_input(char c);
void set_cursor(int x, int y);
void print_command();
void print_time(int curr_time);
void init_track();
void add_sensor_to_queue(int sensor);



void init_globals()
{
	// Initialize Globals
	init_buffer(&char_buffer_com1);
	init_buffer(&char_buffer_com2);
	init_buffer(&read_buffer_com1);
	init_buffer(&read_buffer_com2);
	// init_buffer(&train_buffer);
	char_buffer_com1_size = 0;
	char_buffer_com2_size = 0;
	char_buffer_com1_front = 0;
	char_buffer_com2_front = 0;

	read_buffer_com1_size = 0;
	read_buffer_com2_size = 0;
	read_buffer_com1_front = 0;
	read_buffer_com2_front = 0;

	last_sensor_request_time = 0;
	last_sensor_first_byte = 0;
	last_sensor_last_byte = 0;

	train_buffer_size = 0;
	train_buffer_start = 0;
	train_buffer_end = 0;

	last_train_command_time = 0; // TODO Verify if this is all thats needed
	current_wait_time = 0;

	trains_cnt = 6;
	switches_cnt = 22;
	ran = 0;

	last_time = 0;
	last_print_time = 0;
	last_sensor_time = 0;
	TIMER_SPEED = 508000; //2000;

	time_str[31] = 0;
	command[0] = 0;
	command_len = 0;
	last_command_len = 0;
	MAX_COMMAND_LEN = 64;
	COMMAND_PRINT_HEIGHT = 10;
	exit = 0;
	initializing = 1;

	initialize_track = 1;

	for(int i = 0; i != 16; i++) sensor_states[i] = -1;
	sensor_byte = 0;
	sensor_states_len = 0;
	dump_finished = 1;
	sensor_updated = 1;
	for(int i = 0; i != 128; i++) all_sensor_states[i] = 0;
	switch_updated = 0;

	marklin_dumping = 0;
	bytes_read = 0;
	curr_time = 0;
}

void strcpy(char* dest, char* src)
{
	while(*src)
	{
		*dest = *src;
		src++;
		dest++;
	}
	*dest = 0;
}

int strlen(char* s)
{
	int i = 0;
	while(*s){ i++; s++; }
	return i;
}

int strcmp(char* str1, char*str2)
{
	int r = 0;
	while(*str1 && *str2 && *str1 == *str2){ r++; str1++; str2++; }
	return r;
}

void itobs(int i, char* s)
{
	while(*s) s++;
	s--;

	int curr = i;
	while(curr > 0)
	{
		*s = (curr & 1) ? '1' : '0';
		s--;
		curr = curr >> 1;
	}
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

int put_char( int channel, char c ) {
	// // TODO Write limit check for this
	switch( channel ) {
	case COM1:
		add_element( &char_buffer_com1, c);
		break;
	case COM2:
		add_element( &char_buffer_com2, c);
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

// Try and print a character if the buffer is ready to accept.
// TODO Change to FIFO??
int putc_buffer(int channel)
{
	int *flags, *data;
	char c;

	switch( channel ) 
	{
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}

	// If holding register is empty, write
	if( !( *flags & TXFF_MASK ) )
	{
		// TODO Pick char before?
		// TODO Wrap around circular buffer
		switch( channel ) 
		{
		case COM1:
			if( char_buffer_com1.size > 0)
			{
				c =  remove_element( &char_buffer_com1 );
			}
			else return 0;

			break;
		case COM2:
			if( char_buffer_com2.size > 0)
			{
				c =  remove_element( &char_buffer_com2 );
			}
			else return 0;

			break;
		default:
			return -1;
			break;
		}

		*data = c;
	}

	return 0;
}

// Returns number of characters read
int get_char( int channel, char* c) {
	// // TODO Write limit check for this
	switch( channel ) {
	case COM1:
		if(read_buffer_com1.size > 0) 
		{
			*c = remove_element(&read_buffer_com1);

			return 1;
		}
		else return 0;
	case COM2:
		if(read_buffer_com2.size > 0)
		{
			*c = remove_element(&read_buffer_com2);
			return 1;
		}
		else return 0;
	default:
		return -1;
		break;
	}

	return 0;
}

int getc_buffer( int channel) {
	int *flags, *data;
	unsigned char c;

	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}

	if(FIFO_ON)
	{
		while ( !(*flags & RXFE_MASK ) )
		{
			c = *data;

			switch( channel ) {
			case COM1:
				add_element(&read_buffer_com1, c);
				bytes_read++;
				break;
			case COM2:
				add_element(&read_buffer_com2, c); 
				break;
			}
		}
	}
	else
	{
		if ( *flags & RXFF_MASK ) 
		{
			c = *data;

			switch( channel ) {
			case COM1:
				add_element(&read_buffer_com1, c);
				bytes_read++;
				break;
			case COM2:
				add_element(&read_buffer_com2, c); 
				break;
			}
		}
	}

	return 0;
}

int putstr( int channel, char *str ) {
	while( *str ) 
	{
		if( put_char( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

// TODO FLUSH THE BUFFER
// int flush_buffer( int channel) {
// 	int *flags, *data;
// 	unsigned char c;

// 	switch( channel ) {
// 	case COM1:
// 		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
// 		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
// 		break;
// 	case COM2:
// 		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
// 		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
// 		break;
// 	default:
// 		return -1;
// 		break;
// 	}

// 	while ( *flags & RXFF_MASK ) 
// 	{
// 		c = *data;	
// 		for(int i = 0; i != 10000; i++)
// 		{
// 			int temp = 1;
// 		}
// 	}


// 	return 0;
// }

//////////////////////////// BW REWRITES ///////////////////////////

void putw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) put_char( channel, fc );
	while( ( ch = *bf++ ) ) put_char( channel, ch );
}

// Same code as bw version, but replace bw versions of putc
void format ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			put_char( channel, ch );
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
				ch = bwa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				put_char( channel, va_arg( va, char ) );
				break;
			case 's':
				putw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				bwui2a( va_arg( va, unsigned int ), 10, bf );
				putw( channel, w, lz, bf );
				break;
			case 'd':
				bwi2a( va_arg( va, int ), bf );
				putw( channel, w, lz, bf );
				break;
			case 'x':
				bwui2a( va_arg( va, unsigned int ), 16, bf );
				putw( channel, w, lz, bf );
				break;
			case '%':
				put_char( channel, ch );
				break;
			}
		}
	}
}

void printf( int channel, char *fmt, ... ) {
	va_list va;

	va_start(va,fmt);
	format( channel, fmt, va );
	va_end(va);
}

//////////////////////////// BW REWRITES END ///////////////////////////


char naivegetc(int channel)
{
	int *flags, *data;
	unsigned char c = 0;

	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	// while ( !( *flags & RXFF_MASK ) ) ;
	if ( ( *flags & RXFF_MASK ) ) 
		c = *data;
	return c;
}

void cursor_upper_left()
{
	putstr(COM2, "\033[H");
}

void clear_screen()
{
	putstr(COM2, "\033[2J");
}

void init_clock()
{
	// Ensure clock is disabled first
	int* config = (int *)( TIMER3_BASE + CRTL_OFFSET );
	*config = (*config) & ~ENABLE_MASK;
	

	// write the period to the load register
	int* load = (int *)( TIMER3_BASE + LDR_OFFSET );
	*load = INT_MAX;

	
	// Enable and initialize.
	int c = *config;
	c |= ENABLE_MASK; // Enable the timer
	// c &= ~MODE_MASK; // Set mode to 0
	c |= MODE_MASK;
	// c |= CLKSEL_MASK; // Set to 508 kHz

	if(TIMER_SPEED == 2000) c &= ~CLKSEL_MASK; // Set to 2 kHz
	else if(TIMER_SPEED == 508000) c |= CLKSEL_MASK; // Set to 508 kHz
	*config = c;
}

int get_time()
{
	// return 83;
	int* time = (int *)( TIMER3_BASE + VAL_OFFSET );
	int result = *time;
	return INT_MAX - result;
}


float tics_to_seconds(int tics)
{
	return ((float)tics)/((float)TIMER_SPEED);
}

int seconds_to_tics(float seconds)
{
	return TIMER_SPEED*seconds;
}


void command_append(char c)
{
	// Maximum command size
	if(command_len == MAX_COMMAND_LEN - 1) return;

	// TODO Bound Check
	command[command_len] = c;
	command_len++;
	command[command_len] = 0;
}

void command_popback()
{
	if(command_len == 0) return;
	// TODO Bound Check
	command_len--;
	command[command_len] = 0;
}

void command_clear()
{ 
	command[0] = 0; 
	command_len = 0;
	last_command_len = 0;
	
	// Clear the printed output
	char cleared[MAX_COMMAND_LEN];
	for(int i = 0; i < MAX_COMMAND_LEN; i++) cleared[i] = ' ';
	cleared[MAX_COMMAND_LEN - 1] = 0;

	set_cursor(command_len, COMMAND_PRINT_HEIGHT);
	putstr(COM2, cleared);
}

void printi(int i)
{
	char str[32];
	itos(i, str);
	putstr(COM2, str);
}

void add_train_command(struct train_command command_id, int time)
{
	train_commands[train_buffer_end] = command_id;
	train_times[train_buffer_end] = time;
	if(train_buffer_size != BUFFER_SIZE) train_buffer_size++;

	train_buffer_end = (train_buffer_end + 1)%BUFFER_SIZE;
}

// Handles timing, run as fast as possible
void process_train_command(int curr_time)
{
	// train_times are the MINIMUM time to wait until the next command, may be a bit longer
	if(train_buffer_size && curr_time > last_train_command_time + current_wait_time && !marklin_dumping)
	{
		// printi(current_wait_time);
		current_wait_time = train_times[train_buffer_start];
		struct train_command current_command = train_commands[train_buffer_start];
		int first_time;
		switch(current_command.command_id)
		{
			case 1: // trainspeed
				put_char( COM1,  current_command.arg2);
				put_char( COM1,  current_command.arg1);
				break;
			case 2: // turn solenoid
				// arg1 is switch num
				// arg2 is switch dir
				put_char( COM1, current_command.arg2);
				put_char( COM1, current_command.arg1);
				break;
			case 3: // turn off solenoid
				put_char( COM1, 32);
				break;
			case 4: // end initialization
				set_cursor(1, 2);
				char intializing_text[] = "               ";
				putstr(COM2, intializing_text);
				initializing = 0;
				// for(int i = 0; i != 16; i++) sensor_states[i] = -1;
				break;

			case 5: // Dump sensor data
				put_char(COM1, 128 + 5);
				marklin_dumping = 1;
				last_sensor_request_time = get_time();
				break;
			case 6: // GO 
				put_char( COM1, 96);
				break;
		}

		train_buffer_start = (train_buffer_start + 1)%BUFFER_SIZE;
		train_buffer_size--;
		last_train_command_time = curr_time;
	}
}

// Parses from str pointer to space or end of string 
int stoi(char* s)
{
	// TODO Error check?
	int r = 0;
	while(*s && *s <= '9' && *s >= '0')
	{
		r *= 10;
		r += *s - '0';
		s++;
	}

	return r;
}

void switch_track(int s_number, char dir)
{
	if(dir == 'S')
	{
		add_train_command( create_train_command(2, s_number, 33), seconds_to_tics(0.200));
		switch_states[s_number] = 'S';
	}
	else if(dir == 'C')
	{
		add_train_command( create_train_command(2, s_number, 34), seconds_to_tics(0.200));
		switch_states[s_number] = 'C';
	}
	add_train_command( create_train_command(3, 0, 0), seconds_to_tics(0.150));
	switch_updated = 1;
}

void process_command()
{	
	if (command_len == 0) return;
	if (command_len == 1 && command[0] == 'q') exit = 1;

	char* command_p = command;

	if (command_len == 1) return;
	if (strcmp("tr ", command) == 3) 
	{
		// TODO Error Check?
		command_p += 3;
		while(*command_p && *command_p == ' ') command_p++;
		int t_number = stoi(command_p);
		while(*command_p && *command_p != ' ') command_p++;
		while(*command_p && *command_p == ' ') command_p++;
		int t_speed = stoi(command_p);

		add_train_command( create_train_command(1, t_number, t_speed), seconds_to_tics(0.15));
		train_speeds[t_number] = t_speed;

	}
	if (strcmp("rv ", command) == 3) 
	{
		command_p += 3;
		while(*command_p && *command_p == ' ') command_p++;
		int t_number = stoi(command_p);

		// TODO Make take old speed
		int old_speed = train_speeds[t_number];
		int slow_down_time = (((old_speed & 15 )+1)/4) + 1;
		add_train_command( create_train_command(1, t_number, 0), seconds_to_tics(slow_down_time));
		// add_train_command( create_train_command(1, t_number, 0), seconds_to_tics(4));
		// Leave train speed the same as of last command 
		add_train_command( create_train_command(1, t_number, 15), seconds_to_tics(0.1));
		add_train_command( create_train_command(1, t_number, old_speed), seconds_to_tics(0.15));
	}
	if (strcmp("sw ", command) == 3) 
	{
		command_p += 3;
		while(*command_p && *command_p == ' ') command_p++;
		int s_number = stoi(command_p);
		while(*command_p && *command_p != ' ') command_p++;
		while(*command_p && *command_p == ' ') command_p++;

		if(*command_p == 'C' || *command_p == 'S') switch_track(s_number, *command_p);
	}
}

void process_input(char c)
{
	switch(c)
	{
		case 8: // Backspace
		command_popback();
		break;

		case 27: // ESC
		exit = 1;
		break;

		case 13: // Return pressed
		process_command();
		command_clear();
		break;

		default:
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || (c >= '0' && c <= '9')) command_append(c);
		break;
	}
}

// TODO PRINT AT BOTTOM AND CONTINUE
void set_cursor(int x, int y)
{
	char x_str[32];
	char y_str[32];

	itos(x, x_str);
	itos(y, y_str);

	printf(COM2, "\033[%s;%sH", y_str, x_str);
}

void print_command()
{
	if(command_len == last_command_len) return;
	else if(command_len > last_command_len)
	{
		// Make cursor go to bottom left.
		set_cursor(command_len, COMMAND_PRINT_HEIGHT);
		put_char(COM2, command[command_len-1]);
	}
	else
	{
		// Make cursor go to bottom left.
		set_cursor(1+command_len, COMMAND_PRINT_HEIGHT);
		put_char(COM2, ' ');
	}
}

void print_time(int curr_time)
{
	// Create time string
	cursor_upper_left();
	float seconds_float = tics_to_seconds(curr_time);
	int tenths = ((int)(seconds_float*10))%10;
	int seconds = ((int)seconds_float)%60;
	int minutes = seconds_float/60;

	char tenths_str[8];
	char seconds_str[8];
	char minutes_str[8];

	itos(tenths, tenths_str);
	itos(seconds, seconds_str);
	itos(minutes, minutes_str);

	// If seconds is 1 digit, add zero in front
	if(strlen(seconds_str) == 1)
	{
		seconds_str[2] = 0;
		seconds_str[1] = seconds_str[0];
		seconds_str[0] = '0';
	}

	printf(COM2, "%s:%s.%s\n\r\n\r", minutes_str, seconds_str, tenths_str);
}

void init_track()
{
	// Flush buffer
	int curr_time = get_time();
	while(get_time() - curr_time < seconds_to_tics(0.5)) getc_buffer(COM1);
	init_buffer(&read_buffer_com1);

	for(int i = 0; i < trains_cnt; i++)
	{ 
		// For safety allow turning off track initializaiton when testing.
		if(initialize_track) add_train_command( create_train_command(1, trains[i], 0), seconds_to_tics(0.15));
		train_speeds[trains[i]] = 0; 
	}
	for(int i = 0; i < switches_cnt; i++)
	{ 
		// add_train_command( create_train_command(2, switches[i], 34), seconds_to_tics(0.2));
		if(initialize_track) add_train_command( create_train_command(2, switches[i], 33), seconds_to_tics(0.2)); // Set straight
		switch_states[switches[i]] = 'S'; 
	}
	add_train_command( create_train_command(3, 0, 0), seconds_to_tics(0.3));

	// Send Go Command
	// Its not needed to send go, but this eliminates the possibility of a sensor read happening 
	// directly after a stop solonoid command that didnt stop any solenoids (this causes Marklin 
	// to stop reading sensor commands).
	// add_train_command(create_train_command(6,0,0), seconds_to_tics(0.15));
	// if(initialize_track) add_train_command( create_train_command(1, 1, 0), seconds_to_tics(0.15));

	switch_updated = 1;
}

void add_sensor_to_queue(int sensor)
{
	for(int i = 15; i != 0; i--) sensor_states[i] = sensor_states[i-1];
	sensor_states[0] =  sensor;
}



// baud rate 2400
// stop bits 2
// data bits 8
// no parity
// 191 low byte for 2400

int main( int argc, char* argv[] ) {
	// TODO Send go 96
	int trains_array[] = {1, 24, 58, 74, 78, 79};
	// List initializing past 16 items uses a memcpy
	// int switches_array[] = 
	// {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
	int switches_array[22];
	for(int i = 0; i != 18; i++) switches_array[i] = i + 1;
	switches_array[18] = 0x99;
	switches_array[19] = 0x9A;
	switches_array[20] = 0x9B;
	switches_array[21] = 0x9C;

	trains = trains_array;
	switches = switches_array;
	init_globals();

	bwsetspeed( COM2, 115200 );
	if(FIFO_ON) bwsetfifo( COM2, ON );
	else bwsetfifo( COM2, OFF );

	bwsetspeed( COM1, 2400 );
	if(FIFO_ON) bwsetfifo( COM1, ON );
	else bwsetfifo( COM1, OFF );
	bwtraininitialize( COM1 );

	init_clock();

	cursor_upper_left();
	clear_screen();
	putstr(COM2, "\033[?25l"); // Hide the cursor

	init_track();

	// Signal track is finished intializing.
	add_train_command( create_train_command(4, 0, 0), 0);

	set_cursor(1, 2);
	putstr(COM2, "INITIALIZING...");

	int last_loop_time = get_time();
	int max_loop_time = 0;
	int total_loop_time = 0;
	int loops = 0;

	int max_first_byte_time = 0;
	int max_last_byte_time = 0;

	for (;;) 
	{
		char keyboard_input;
		int keyboard_pressed = get_char(COM2, &keyboard_input);

		

		if(exit && ! initializing)
		{
			// TODO Clear screen
			break;
		}

		// Process and print actions for keystrokes
		if (keyboard_pressed)
		{
			process_input(keyboard_input);

			// Display current command
			if(last_command_len != command_len) print_command();
			last_command_len = command_len;
		} 

		// Get the current tics since start
		curr_time = get_time();

		float loop_time = tics_to_seconds(curr_time - last_loop_time);
		loop_time = loop_time * 1000;

		if((int)loop_time > max_loop_time) max_loop_time = (int)loop_time;
		loops++;
		total_loop_time += loop_time;
		

		last_loop_time = curr_time;

		// if(curr_time - last_sensor_time > seconds_to_tics(0.15) && dump_finished == 1)
		if(marklin_dumping == 0 && dump_finished)
		{
			// Dump sensor memory
			add_train_command( create_train_command(5, 0, 0), 0);

			// last_sensor_time = curr_time; // TODO Add back!?!
			dump_finished = 0;
		}

		

		if(marklin_dumping)
		{
		char char_read;
		int char_cnt_read = get_char(COM1, &char_read);
 
		if(char_cnt_read > 0)
		{
			if(sensor_byte == 0)
			{
				last_sensor_first_byte = get_time();
				if(last_sensor_first_byte - last_sensor_request_time > max_first_byte_time) 
					max_first_byte_time = last_sensor_first_byte - last_sensor_request_time;

			} 


			sensor_bytes[sensor_byte] = char_read;

			if(sensor_byte == 9) // A full dump has been read, process it!
			{
				last_sensor_last_byte = get_time();
				// int max_last_byte_time = 0;

				if(last_sensor_last_byte - last_sensor_request_time > max_last_byte_time) 
					max_last_byte_time = last_sensor_last_byte - last_sensor_request_time;

				for( int i = 0; i != 10; i+=2)
				{
					char byte1 = sensor_bytes[i];
					char byte2 = sensor_bytes[i+1];
					int data = byte1;
					data = data << 8;
					data += byte2;

					int sensor_bank = i/2;

					for(int i = 16; i != 0; i--)
					{
						int sensor = sensor_bank * 16 + i;
						if(data & 1) 
						{
							if(!all_sensor_states[sensor])
							{
								sensor_updated = 1;
								add_sensor_to_queue(sensor_bank * 16 + (i-1)); 
								all_sensor_states[sensor] = 1;
							}
						}
						else
						{
							all_sensor_states[sensor] = 0;
						}
						data = data >> 1;
					}
				}
				dump_finished = 1;
				marklin_dumping = 0;

			}

			sensor_byte++;
			sensor_byte %= 10;
		}

		}

		// Print Sensors
		if(sensor_updated && !initializing)
		{
			set_cursor(1, 8);
			putstr(COM2, "LAST 16 SENSOR HITS: ");
			set_cursor(22, 8);
			char sensor[32];

			for(int i = 0; i != 16; i++)
			{
				if(sensor_states[i] == -1) break;
				itos(sensor_states[i], sensor);

				char sensor_str[4];
				sensor_str[3] = 0;
				char* number_part =sensor_str + 1;

				sensor_str[0] = 'A' + sensor_states[i]/16;
				itos(sensor_states[i]%16+1, number_part);

				putstr(COM2, sensor_str);
				put_char(COM2, ' ');
			}

			sensor_updated = 0;
		}

		// Print switches
		if(switch_updated && !initializing)
		{
			set_cursor(1, 4);
			putstr(COM2, "SWITCHES:");

			set_cursor(1, 5);
			char switch_number[32];

			int spacing = 5;

			for(int i = 0; i != switches_cnt; i++)
			{
				itos(switches[i], switch_number);
				int space_left = spacing - strlen(switch_number);

				putstr(COM2, switch_number);
				for (int i = 0; i != space_left; i++) put_char(COM2, ' ');
			}

			set_cursor(1, 6);
			for(int i = 0; i != switches_cnt; i++)
			{
				char switch_state = switch_states[switches[i]];

				put_char(COM2, switch_state);
				for (int i = 0; i != spacing - 1; i++) put_char(COM2, ' ');
			}

			switch_updated = 0;
		}

		// Print time
		if(curr_time - last_print_time > seconds_to_tics(0.03))
		{
			print_time(curr_time);

			last_print_time = curr_time;

			// set_cursor(1,9);
			// putstr(COM2, "      ");
			// set_cursor(1,9);
			// printi(max_loop_time);
			// printi(tics_to_seconds(max_last_byte_time)*1000);
			// printi(tics_to_seconds(max_first_byte_time)*1000);
			
			// STATS
			// Max loop time: 7 ms
			// Max first byte retrieve time: 23 ms
			// Max last byte retrieve time: 67 ms


			// FOR DEBUG
			// set_cursor(1,3);
			// printi(char_buffer_com1.start);
			// set_cursor(1,4);
			// printi(read_buffer_com1.start);
			
			// set_cursor(1,10);
			// printi(char_buffer_com1.size);
			// set_cursor(1,11);
			// printi(read_buffer_com1.size);
			// set_cursor(1,12);
			// printi(sensor_byte);


			// set_cursor(1,13);
			// printi(bytes_read);
			// set_cursor(1,14);
			// if(curr_time - last_loop_time > max_time) max_time = curr_time - last_loop_time;
			// printi(max_time);//(curr_time - last_loop_time)*2000/508000);
			// last_loop_time = curr_time;
			// set_cursor(1,15);
			// printi(train_buffer_size); // 1657

			// set_cursor(1,16);
			// printi(train_buffer_front); // 1657

			// set_cursor(1,17);
			// printi(ran); // 1657

			// // set_cursor(1,16);
			// // printi(char_buffer_com2_front);
			// // set_cursor(1,17);
			// // printi(read_buffer_com2_front);
			
			// // set_cursor(1,18);
			// // printi(char_buffer_com2_size);
			// // set_cursor(1,19);
			// // printi(read_buffer_com2_size);
			
		}

		process_train_command(curr_time);
		
		// Write characters from buffers if their respective channels arent busy
		// Change printing to fifo?? -> NO
		putc_buffer(COM1);
		putc_buffer(COM2);

		// Read characters to buffers if their respective channels have received data
		getc_buffer(COM1);
		getc_buffer(COM2);

		last_time = curr_time;
	}
	

	return 0;
}

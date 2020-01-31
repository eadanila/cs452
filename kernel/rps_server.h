void rps_server();

#define ROCK 'r'
#define PAPER 'p'
#define SCISSORS 's'

#define WIN 0
#define LOSE 1
#define TIE 2

#define OPPONENT_QUIT 3
#define NOT_SIGNED_UP 4

void Signup();
int Play(char move);
void Quit();
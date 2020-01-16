#define SYSCALL_YIELD 1

void Yield();
int syscall(int id);

// Put "b scream" anywhere in assembly for basic debugging
void scream();
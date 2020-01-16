#define SYSCALL_YIELD 1

void init_kernel();
void Yield();
int syscall(int id);

// Put "b scream" anywhere in assembly for basic debugging
void scream();
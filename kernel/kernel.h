void init_kernel();

int Create(int priority, void (*function)());
void Yield();
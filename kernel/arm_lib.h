extern int enter_kernel();
extern int enter_user(void *task_stack);
extern int syscall(int syscall_id);
extern int init_task(void *stack_ptr, void (*f)(void)); // returns new top of stack

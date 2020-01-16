extern int enter_kernel();
extern void enter_user(int task_stack);
extern void software_interupt(int syscall_id);
extern int init_task(int stack_ptr, int function); // returns new top of stack
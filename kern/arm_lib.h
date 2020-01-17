// syscall(call_id):
//  enter_kernel_mode(); => disable interrupts
//  void *stack_pointer = save_task_tate();
//  task[n].sp = stack_pointer;
//  int result = syscall_handler(call_id); 
//  task[n].result = result;
//  int new_task_id = schedule(); (OR resume old task)
//  restore_task_state(tasks[new_task_id].sp);
//  enter_user_mode() => enable interrupts
//  return tasks[new_task_id].result;

extern void* save_task_state();
extern int syscall_handler(int call_id);
extern void restore_task_state(void* task_stack);

extern void software_interupt(int syscall_id);
extern int init_task(int stack_ptr, int function); // returns new top of stack

#ifndef TASK_H
#define TASK_H

// States
#define TASK_INVALID -100
#define TASK_READY 0
#define TASK_RUNNING 1

#define TASK_SEND_WAIT 2
#define TASK_RECV_WAIT 3
#define TASK_RPLY_WAIT 4

#define TASK_AWAIT 5

#define TASK_ZOMBIE 7

#define PARENT_DEAD -10
#define PARENT_ZOMBIE -11

// t_id, p_id, priority, stack_base, and pc should be set when a
// task is created and never changed
// Support changing only the state, exit_code, next, and 
// stack_pointer
struct task {
    int t_id;
    int p_id;
    int priority;
    int state;
    int exit_code;
    unsigned int *stack_pointer;
    unsigned int *stack_base;
};
typedef struct task Task;

// Require: id is a valid task
//  Return: task state in set of valid states
int get_task_state(int id);

// Require: id is a valid task, valid state
void set_task_state(int id, int state);

// Require: id is a valid task that has exited
//  Return: the return code of a task that has exited
int get_task_exit_code(int id);

// Require: id is a valid task
void set_task_exit_code(int id, int code);

// Require: id is a valid task
//  Return: task's stack_pointer for that id, where 
//  stack_base - MAX_STACK_SIZE < stack_pointer <= stack_base
unsigned int *get_task_stack_pointer(int id);

// Require: id is a valid task, and
// stack_base - MAX_STACK_SIZE < sp < stack_base
void set_task_stack_pointer(int id, unsigned int *sp);

// Initializes the task_list by setting relevant fields for all items
// Sets t_id to array index
// Sets p_id to TASK_INVALID
void init_task_list(void);

// Require: id is a valid task
// Sets the current running task to the provided id
// The provided id MUST describe a valid task
void set_running_task(int id);

//  Return: a valid task ID, or 
//          TASK_INVALID if no task is running
int get_running_task(void);

// Require: id is a valid task
//  Return: the task structure for that id
Task get_task_by_id(int id);

// Require: p_id is a valid task, pri is in [0, MIN_PRIORITY)
//  Return: ID of newly allocated task having TASK_READY state, or 
//          -1 if ID could not be allocated
// Selects a task ID and sets its parent, priority, and function
// from parameters, and state to TASK_READY. Also selects a stack
// base address, initializes the stack, and sets the stack pointer
// accordingly.
int allocate_task(int p_id, int pri);

// Require: id is a valid task
// Frees a task by setting its state to TASK_INVALID so that
// it can be allocated later
void free_task(int id);

int get_active_tasks_count(void);

//  Return: 0 if id is not in [0, MAX_ALLOWED_TASKS) or if
//          id describes a task with TASK_INVALID state
int is_valid_task(int id);

#endif


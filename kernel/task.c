#include "task.h"
#include "logging.h"

int running_task;
task task_list[MAX_TASKS_ALLOWED];

int __is_defined_task_state(int state) {
    int valid = (state == TASK_INVALID);
    valid = valid || (state == TASK_READY);
    valid = valid || (state == TASK_RUNNING);
    valid = valid || (state == TASK_ZOMBIE);

    return valid;
}

// Seeks through task_list for the next available ID
// An ID is considered available if the task is invalid
int __get_next_available_id() {
    for (int i = 0; i < MAX_TASKS_ALLOWED; i++) {
        if (task_list[i].state == TASK_INVALID)
            return task_list[i].t_id;
    }

    WARN("%s has run out of valid task IDs", KNAME);
    return TASK_INVALID;
}

int get_task_state(int id) {
    assert(is_valid_task(id));
    assert(__is_defined_task_state(task_list[id].state));

    return task_list[id].state;
}

void set_task_state(int id, int state) {
    assert(is_valid_task(id));
    assert(__is_defined_task_state(state));
    
    task_list[id].state = state;

    if (state == TASK_RUNNING) {
        set_running_task(id);
        return ;
    }

    if (id == running_task) {
        running_task = -1;
        return ;
    }
}

int get_task_exit_code(int id) {
    assert(is_valid_task(id));
    assert(task_list[id].state == TASK_ZOMBIE);

    return task_list[id].exit_code;
}

void set_task_exit_code(int id, int code) {
    assert(is_valid_task(id));

    task_list[id].exit_code = code;
}

int get_task_next_id(int id) {
    assert(is_valid_task(id));

    int next = task_list[id].next_id;
    assert(is_valid_task(next) || next == TASK_INVALID);

    return next;
}

void set_task_next_id(int id, int next_id) {
    assert(is_valid_task(id));
    assert(is_valid_task(next_id));
    assert(id != next_id);

    task_list[id].next_id = next_id;
}    

void unset_task_next_id(int id) {
    assert(is_valid_task(id));

    task_list[id].next_id = TASK_INVALID;
}

unsigned int *get_task_stack_pointer(int id) {
    assert(is_valid_task(id));

    return task_list[id].stack_pointer;
}

void set_task_stack_pointer(int id, unsigned int *sp) {
    assert(is_valid_task(id));
    assert(sp <= task_list[id].stack_base);
    assert(sp > task_list[id].stack_base - TASK_MEMORY_SIZE);

    task_list[id].stack_pointer = sp;
}

void init_task_list(void) {
    running_task = -1;
    for (int i = 0; i < MAX_TASKS_ALLOWED; i++) {
        task_list[i].t_id = i;
        task_list[i].p_id = TASK_INVALID;
        task_list[i].next_id = TASK_INVALID;
        task_list[i].state = TASK_INVALID;
    }
}

void set_running_task(int id) {
    assert(is_valid_task(id));
    assert(running_task == -1);

    running_task = id;
}

int get_running_task(void) {
    assert(is_valid_task(running_task) || running_task == -1);

    return running_task;
}

task get_task_by_id(int id) {
    assert(is_valid_task(id));

    return task_list[id];
}

int allocate_task(int p_id, int pri, void (*pc)(void)) {
    assert(is_valid_task(p_id) || p_id == -1);
    assert(pri >= 0 && pri < MIN_PRIORITY);

    int id = __get_next_available_id();
    if (id == TASK_INVALID)
        return OUT_OF_TASK_DESCRIPTORS;

    task_list[id].p_id = p_id;
    task_list[id].priority = pri;
    task_list[id].pc = pc;
    task_list[id].stack_base = (unsigned int *)(MEMORY_START + id*TASK_MEMORY_SIZE);
    task_list[id].state = TASK_READY;

    return task_list[id].t_id;
}

void free_task(int id) {
    assert(is_valid_task(id));

    task_list[id].state = TASK_INVALID;
}

int is_valid_task(int id) {
    return (id < MAX_TASKS_ALLOWED) && task_list[id].state != TASK_INVALID;
}
    

int MyTid(void) {
    assert(is_valid_task(running_task));

    return task_list[running_task].t_id;
}

int MyParentTid(void) {
    assert(is_valid_task(running_task));

    return task_list[running_task].p_id;
}


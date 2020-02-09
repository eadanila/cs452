#include "task.h"
#include "logging.h"
#include "constants.h"

int active_tasks;
int running_task;
Task task_list[MAX_TASKS_ALLOWED];

int __is_defined_task_state(int state) {
    return (state == -100) || (state >= 0 && state <= 5) || (state == 7) || (state >= -11 && state <= -10);
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
    DEBUG("get_task_state(%d)", id);
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

    if (state == TASK_ZOMBIE) {
        for (int i = 0; i < MAX_TASKS_ALLOWED; i++) {
            if (task_list[i].p_id == id) {
                task_list[i].p_id = PARENT_ZOMBIE;
            }
        }
        active_tasks -= 1;
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
    active_tasks = 0;
    running_task = -1;
    for (int i = 0; i < MAX_TASKS_ALLOWED; i++) {
        task_list[i].t_id = i;
        task_list[i].p_id = TASK_INVALID;
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

Task get_task_by_id(int id) {
    assert(is_valid_task(id));

    return task_list[id];
}

int allocate_task(int p_id, int pri) {
    assert(is_valid_task(p_id) || p_id == -1);
    assert(pri >= 0 && pri <= MIN_PRIORITY);

    int id = __get_next_available_id();
    if (id == TASK_INVALID)
        return OUT_OF_TASK_DESCRIPTORS;

    task_list[id].p_id = p_id;
    task_list[id].priority = pri;
    task_list[id].stack_base = (unsigned int *)(MEMORY_START + id*TASK_MEMORY_SIZE);
    task_list[id].state = TASK_READY;

    active_tasks += 1;

    return task_list[id].t_id;
}

void free_task(int id) {
    assert(is_valid_task(id));

    if (task_list[id].state != TASK_ZOMBIE)
        active_tasks -= 1;

    task_list[id].state = TASK_INVALID;
}

int get_active_tasks_count(void) {
    return active_tasks;
}

int is_valid_task(int id) {
    return (id >= 0) && (id < MAX_TASKS_ALLOWED) && task_list[id].state != TASK_INVALID;
}


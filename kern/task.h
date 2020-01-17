struct task {
    int t_id;
    int p_id;
    void *sp;
    void (*fp)(void);
    int result;
};

typedef struct task TASK;

TASK* current_task;

TASK t1;
TASK t2;
int task_select;

#include "timer.h"
#include "constants.h"
#include "logging.h"


unsigned int read_debug_timer(void) {
    return *DEBUG_TIMER_LOW;
}


void stop_debug_timer(void) {
    *DEBUG_TIMER_HIGH = 0x0;
}


void start_debug_timer(void) {
    *DEBUG_TIMER_HIGH = 0x100;
}


void disable_timer(uint timer_id) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3 || timer_id == TIMER_TC4);

    switch (timer_id) {
    case TIMER_TC1:
        *TIMER_1_CONTROL_REGISTER &= ~TIMER_ENABLE;
        break;
    case TIMER_TC2:
        *TIMER_2_CONTROL_REGISTER &= ~TIMER_ENABLE;
        break;
    case TIMER_TC3:
        *TIMER_3_CONTROL_REGISTER &= ~TIMER_ENABLE;
        break;
    case TIMER_TC4:
        stop_debug_timer();
        break;
    default:
        FATAL("Invalid timer ID: %d", timer_id);
    }
}


void enable_timer(uint timer_id) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3 || timer_id == TIMER_TC4);

    switch (timer_id) {
    case TIMER_TC1:
        *TIMER_1_CONTROL_REGISTER |= TIMER_ENABLE;
        break;
    case TIMER_TC2:
        *TIMER_2_CONTROL_REGISTER |= TIMER_ENABLE;
        break;
    case TIMER_TC3:
        *TIMER_3_CONTROL_REGISTER |= TIMER_ENABLE;
        break;
    case TIMER_TC4:
        start_debug_timer();
        break;
    default:
        FATAL("Invalid timer ID: %d", timer_id);
    }
}


void clear_timer(uint timer_id) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3);

    switch (timer_id) {
    case TIMER_TC1:
        *TIMER_1_CLEAR_REGISTER = 0;
        break;
    case TIMER_TC2:
        *TIMER_2_CLEAR_REGISTER = 0;
        break;
    case TIMER_TC3:
        *TIMER_3_CLEAR_REGISTER = 0;
        break;
    case TIMER_TC4:
    default:
        FATAL("Invalid timer ID for clear_timer: %d", timer_id);
    }
}


void set_timer_mode(uint timer_id, uint mode) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3);
    assert(mode == 0 || mode == 1);

    volatile uint *timer_control;

    switch (timer_id) {
    case TIMER_TC1:
        timer_control = TIMER_1_CONTROL_REGISTER;
        break;
    case TIMER_TC2:
        timer_control = TIMER_2_CONTROL_REGISTER;
        break;
    case TIMER_TC3:
        timer_control = TIMER_3_CONTROL_REGISTER;
        break;
    case TIMER_TC4:
    default:
        FATAL("Invalid timer ID for set_timer_mode: %d", timer_id);
        return ;
    }

    switch (mode) {
    case 0:
        *timer_control &= ~(TIMER_MODE);
        break;
    case 1:
        *timer_control |= TIMER_MODE;
        break;
    default:
        FATAL("Invalid timer mode: %d", mode);
    }
}


void set_timer_clock(uint timer_id, uint clock) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3);
    assert(clock == 0 || clock == 1);

    volatile uint *timer_control;

    switch (timer_id) {
    case TIMER_TC1:
        timer_control = TIMER_1_CONTROL_REGISTER;
        break;
    case TIMER_TC2:
        timer_control = TIMER_2_CONTROL_REGISTER;
        break;
    case TIMER_TC3:
        timer_control = TIMER_3_CONTROL_REGISTER;
        break;
    case TIMER_TC4:
    default:
        FATAL("Invalid timer ID for set_timer_clock: %d", timer_id);
        return ;
    }

    switch (clock) {
    case 0:
        *timer_control &= ~(TIMER_CLKSEL);
        break;
    case 1:
        *timer_control |= TIMER_CLKSEL;
        break;
    default:
        FATAL("Invalid timer mode: %d", clock);
    }
}


void set_timer_load_value(uint timer_id, uint value) {
    assert(((timer_id == TIMER_TC1 || timer_id == TIMER_TC2) && value <= 0xFFFF) || timer_id == TIMER_TC3);

    switch (timer_id) {
    case TIMER_TC1:
        *TIMER_1_LOAD_REGISTER = value;
        break;
    case TIMER_TC2:
        *TIMER_2_LOAD_REGISTER = value;
        break;
    case TIMER_TC3:
        *TIMER_3_LOAD_REGISTER = value;
        break;
    case TIMER_TC4:
    default:
        FATAL("Invalid timer ID for set_timer_load_value: %d", timer_id);
    }
}


uint read_timer(uint timer_id) {
    assert(timer_id == TIMER_TC1 || timer_id == TIMER_TC2 || timer_id == TIMER_TC3 || timer_id == TIMER_TC4);

    switch(timer_id) {
    case TIMER_TC1:
        return *TIMER_1_VALUE_REGISTER;
    case TIMER_TC2:
        return *TIMER_2_VALUE_REGISTER;
    case TIMER_TC3:
        return *TIMER_3_VALUE_REGISTER;
    case TIMER_TC4:
        return read_debug_timer();
    default:
        FATAL("Invalid timer ID for read_timer: %d", timer_id);
        return 0;
    }
}


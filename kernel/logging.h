#ifndef LOGGING_H
#define LOGGING_H 

#include <bwio.h>

#define LOGLEVEL_NONE 0  // logging off

// Sending a fatal message implies execution cannot continue.
// The same is true for failed asserts. In both of these cases
// we exit the task if the processor is in user mode, or panic
// the kernel if the processor is not in user mode.
#define LOGLEVEL_FATAL 1 // Log fatal messages only
#define LOGLEVEL_ERROR 2 // above + error messages
#define LOGLEVEL_WARN 3 // above + warning messages
#define LOGLEVEL_LOG 4 // above + general logs
#define LOGLEVEL_DEBUG 5 // above + debug messages

#ifndef VERBOSITY
    #define VERBOSITY LOGLEVEL_NONE
#endif

#define PRINTLOG(type, msg, ...) { \
    bwprintf(COM2, "%s:<%s:%d> - ", type, __FILE__, __LINE__); \
    bwprintf(COM2, msg, ##__VA_ARGS__); \
    bwprintf(COM2, "\r\n"); \
}


#if VERBOSITY >= LOGLEVEL_FATAL
    #define FATAL(msg, ...) { \
        PRINTLOG("FATAL", msg, ##__VA_ARGS__) \
        if (!user_mode()) panic(); \
        syscall(SYSCALL_EXIT); \
    }

    #define assert(cond) { \
        if (!(cond)) { \
            PRINTLOG("FATAL - ASSERT FAILED", #cond) \
            if (!user_mode()) panic(); \
            syscall(SYSCALL_EXIT); \
        } \
    }
#else
    #define FATAL(msg, ...) { \
        if (!user_mode()) panic(); \
        syscall(SYSCALL_EXIT); \
    }

    #define assert(cond, ...) { \
        if (!(cond)) { \
            if (!user_mode()) panic(); \
            syscall(SYSCALL_EXIT); \
        } \
    }
#endif


#if VERBOSITY >= LOGLEVEL_ERROR
    #define ERROR(msg, ...) { PRINTLOG("ERROR", msg, ##__VA_ARGS__) }
#else
    #define ERROR(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_WARN
    #define WARN(msg, ...) { PRINTLOG("WARN", msg, ##__VA_ARGS__) }
#else
    #define WARN(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_LOG
    #define LOG(msg, ...) { PRINTLOG("LOG", msg, ##__VA_ARGS__) }
#else
    #define LOG(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_DEBUG
    #define DEBUG(msg, ...) { PRINTLOG("DEBUG", msg, ##__VA_ARGS__) }
#else
    #define DEBUG(msg, ...) { }
#endif


#define print(msg, ...) { bwprintf(COM2, msg, ##__VA_ARGS__); }


#endif


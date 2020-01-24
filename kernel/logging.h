#ifndef LOGGING_H
#define LOGGING_H 

#include <bwio.h>
#include "arm_lib.h"
#include "syscall.h"

#define LOGLEVEL_NONE 0  // logging off

#define FATAL_RAINBOW 0 // Alleviate pain caused by fatal errors

// Sending a fatal message implies execution cannot continue.
// The same is true for failed asserts. In both of these cases
// we exit the task if the processor is in user mode, or panic
// the kernel if the processor is not in user mode.
#define LOGLEVEL_FATAL 1 // Log fatal messages only
#define LOGLEVEL_ERROR 2 // above + error messages
#define LOGLEVEL_WARN 3 // above + warning messages
#define LOGLEVEL_LOG 4 // above + general logs
#define LOGLEVEL_DEBUG 5 // above + debug messages

// Foreground Colors
#define BLACK_TEXT   "\033[30m"
#define RED_TEXT     "\033[31m"
#define GREEN_TEXT   "\033[32m"
#define YELLOW_TEXT  "\033[33m"
#define BLUE_TEXT    "\033[34m"
#define MAGENTA_TEXT "\033[35m"
#define CYAN_TEXT    "\033[36m"
#define WHITE_TEXT   "\033[37m"

// Background Colors
#define BLACK_BACKGROUND   "\033[40m"
#define RED_BACKGROUND     "\033[41m"
#define GREEN_BACKGROUND   "\033[42m"
#define YELLOW_BACKGROUND  "\033[43m"
#define BLUE_BACKGROUND    "\033[44m"
#define MAGENTA_BACKGROUND "\033[45m"
#define CYAN_BACKGROUND    "\033[46m"
#define WHITE_BACKGROUND   "\033[47m"

#define RESET_FORMATTING "\033[0m"


#ifndef VERBOSITY
    #define VERBOSITY LOGLEVEL_NONE
#endif


#define PRINTLOG(type, msg, ...) { \
    bwprintf(COM2, "%s:<%s:%d> - ", type, __FILE__, __LINE__); \
    bwprintf(COM2, msg, ##__VA_ARGS__); \
    bwprintf(COM2, "\r\n"); \
}


// Prints with colored text, resets formatting
#define PRINTLOGCOLOR(text_color, type, msg, ...) { \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, text_color); \
    bwprintf(COM2, "%s:<%s:%d> - ", type, __FILE__, __LINE__); \
    bwprintf(COM2, msg, ##__VA_ARGS__); \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, "\r\n"); \
}


// Prints with colored text and background, resets formatting
#define PRINTLOGFULLCOLOR(text_color, background_color, type, msg, ...) { \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, background_color); \
    bwprintf(COM2, text_color); \
    bwprintf(COM2, "%s:<%s:%d> - ", type, __FILE__, __LINE__); \
    bwprintf(COM2, msg, ##__VA_ARGS__); \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, "\r\n"); \
}

// Prints with rainbow text, resets formatting
#define PRINTLOGRAINBOW(type, msg, ...){ \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, RED_TEXT); \
    bwprintf(COM2, "%s:<%s:%d> - ", type, __FILE__, __LINE__); \
    for(int i = 0; msg[i]; i++) { \
        switch(i%6) \
        { \
            case 0: bwprintf(COM2, RED_TEXT); break; \
            case 1: bwprintf(COM2, YELLOW_TEXT); break; \
            case 2: bwprintf(COM2, GREEN_TEXT); break; \
            case 3: bwprintf(COM2, CYAN_TEXT); break; \
            case 4: bwprintf(COM2, BLUE_TEXT); break; \
            case 5: bwprintf(COM2, MAGENTA_TEXT); break; \
        } \
        /*bwprintf(COM2, "\033[3");*/ \
        /*bwputc(COM2, '0' + i%6 + 1);*/ \
        /*bwprintf(COM2, "m");*/ \
        bwputc(COM2, msg[i]); \
    } \
    bwprintf(COM2, RESET_FORMATTING); \
    bwprintf(COM2, "\r\n"); \
}


#if VERBOSITY >= LOGLEVEL_FATAL
    #define FATAL(msg, ...) { \
        if(FATAL_RAINBOW) { PRINTLOGRAINBOW("FATAL", msg, ##__VA_ARGS__); }\
        else { PRINTLOGFULLCOLOR(BLACK_TEXT, RED_BACKGROUND, "FATAL", msg, ##__VA_ARGS__); }\
        if (!user_mode()) panic(); \
        Exit(); \
    }

    #define assert(cond) { \
        if (!(cond)) { \
            if(FATAL_RAINBOW) { PRINTLOGRAINBOW("FATAL - ASSERT FAILED", #cond); } \
            else { PRINTLOGFULLCOLOR(BLACK_TEXT, RED_BACKGROUND, "FATAL - ASSERT FAILED", #cond); } \
            if (!user_mode()) panic(); \
            Exit(); \
        } \
    }
#else
    #define FATAL(msg, ...) { }

    #define assert(cond, ...) { } 
#endif


#if VERBOSITY >= LOGLEVEL_ERROR
    #define ERROR(msg, ...) { PRINTLOGCOLOR(RED_TEXT, "ERROR", msg, ##__VA_ARGS__) }
#else
    #define ERROR(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_WARN
    #define WARN(msg, ...) { PRINTLOGCOLOR(YELLOW_TEXT, "WARN", msg, ##__VA_ARGS__) }
#else
    #define WARN(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_LOG
    #define LOG(msg, ...) { PRINTLOGCOLOR(WHITE_TEXT, "LOG", msg, ##__VA_ARGS__) }
#else
    #define LOG(msg, ...) { }
#endif


#if VERBOSITY >= LOGLEVEL_DEBUG
    #define DEBUG(msg, ...) { PRINTLOGCOLOR(MAGENTA_TEXT, "DEBUG", msg, ##__VA_ARGS__) }
#else
    #define DEBUG(msg, ...) { }
#endif


#define print(msg, ...) { bwprintf(COM2, msg, ##__VA_ARGS__); }


#endif


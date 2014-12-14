#ifndef _PEANUT_UTILS_H_
#define _PEANUT_UTILS_H_

#include <execinfo.h>
#include <unistd.h>
#define STACK_TRACE_SIZE 20

#define print_stack_trace() { \
    void *array[STACK_TRACE_SIZE]; \
    size_t size = backtrace(array, STACK_TRACE_SIZE); \
    backtrace_symbols_fd(array, size, STDERR_FILENO); \
}

#define PN_ASSERT(x) { \
    if (!(x)) { \
        fprintf(stderr, "Failed: PN_ASSERT(%s)\n", #x); \
        print_stack_trace(); \
        assert(x); \
    }\
}

#define PN_FAIL(x) PN_ASSERT(0 && (x))

#endif//_PEANUT_UTILS_H_

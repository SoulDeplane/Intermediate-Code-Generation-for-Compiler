#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <setjmp.h>

typedef enum {
    ERR_PHASE_LEX = 0,
    ERR_PHASE_SYN,
    ERR_PHASE_SEM,
    ERR_PHASE_ICG,
    ERR_PHASE_SYS
} ErrorPhase;

typedef enum {
    ERR_SEV_WARNING = 0,
    ERR_SEV_ERROR,
    ERR_SEV_FATAL
} ErrorSeverity;

void error_reset_all(void);
void error_set_fatal_jmp(jmp_buf *env);
void error_log(ErrorPhase phase, ErrorSeverity sev, int line, int col, const char *code, const char *message, const char *recovery);
void error_logf(ErrorPhase phase, ErrorSeverity sev, int line, int col, const char *code, const char *recovery, const char *fmt, ...);
int error_has_errors(void);
void error_print_summary(FILE *out);

#endif

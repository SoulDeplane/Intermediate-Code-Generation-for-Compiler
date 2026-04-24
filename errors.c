#include "errors.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static int g_warn_count = 0;
static int g_err_count = 0;
static int g_fatal_count = 0;
static jmp_buf *g_fatal_env = NULL;

static const char *phase_text(ErrorPhase p) {
    switch (p) {
        case ERR_PHASE_LEX: return "LEX";
        case ERR_PHASE_SYN: return "SYN";
        case ERR_PHASE_SEM: return "SEM";
        case ERR_PHASE_ICG: return "ICG";
        case ERR_PHASE_SYS: return "SYS";
        default: return "UNK";
    }
}

static const char *sev_text(ErrorSeverity s) {
    switch (s) {
        case ERR_SEV_WARNING: return "warning";
        case ERR_SEV_ERROR: return "error";
        case ERR_SEV_FATAL: return "fatal";
        default: return "unknown";
    }
}

void error_reset_all(void) {
    g_warn_count = 0;
    g_err_count = 0;
    g_fatal_count = 0;
}

void error_set_fatal_jmp(jmp_buf *env) {
    g_fatal_env = env;
}

void error_log(ErrorPhase phase, ErrorSeverity sev, int line, int col, const char *code, const char *message, const char *recovery) {
    const char *c = code ? code : "UNKNOWN";
    const char *m = message ? message : "No message provided.";
    const char *r = recovery ? recovery : "";

    if (sev == ERR_SEV_WARNING) g_warn_count++;
    else if (sev == ERR_SEV_ERROR) g_err_count++;
    else if (sev == ERR_SEV_FATAL) g_fatal_count++;

    fprintf(stderr, "[%s][%s] line=%d col=%d code=%s: %s",
            sev_text(sev), phase_text(phase), line, col, c, m);
    if (r[0] != '\0') fprintf(stderr, " | recovery: %s", r);
    fputc('\n', stderr);

    if (sev == ERR_SEV_FATAL && g_fatal_env) {
        longjmp(*g_fatal_env, 1);
    }
}

void error_logf(ErrorPhase phase, ErrorSeverity sev, int line, int col, const char *code, const char *recovery, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    error_log(phase, sev, line, col, code, buf, recovery);
}

int error_has_errors(void) { return g_err_count > 0 || g_fatal_count > 0; }

void error_print_summary(FILE *out) {
    fprintf(out, "\n--- Diagnostics Summary ---\n");
    fprintf(out, "Warnings: %d\nErrors: %d\nFatals: %d\n",
            g_warn_count, g_err_count, g_fatal_count);
}

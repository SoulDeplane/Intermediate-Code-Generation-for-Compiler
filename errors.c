#include "errors.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef struct ErrorEntry {
    ErrorPhase phase;
    ErrorSeverity severity;
    int line;
    int col;
    char code[64];
    char message[256];
    char recovery[160];
    struct ErrorEntry *next;
} ErrorEntry;

static ErrorEntry *g_head = NULL;
static ErrorEntry *g_tail = NULL;
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

static void free_list(void) {
    ErrorEntry *p = g_head;
    while (p) {
        ErrorEntry *n = p->next;
        free(p);
        p = n;
    }
    g_head = NULL;
    g_tail = NULL;
}

void error_reset_all(void) {
    free_list();
    g_warn_count = 0;
    g_err_count = 0;
    g_fatal_count = 0;
}

void error_set_fatal_jmp(jmp_buf *env) {
    g_fatal_env = env;
}

void error_log(ErrorPhase phase, ErrorSeverity sev, int line, int col, const char *code, const char *message, const char *recovery) {
    ErrorEntry *e = (ErrorEntry*)calloc(1, sizeof(ErrorEntry));
    if (!e) {
        fprintf(stderr, "[FATAL][SYS] Out of memory while logging an error.\n");
        if (g_fatal_env) longjmp(*g_fatal_env, 1);
        exit(EXIT_FAILURE);
    }

    e->phase = phase;
    e->severity = sev;
    e->line = line;
    e->col = col;
    snprintf(e->code, sizeof(e->code), "%s", code ? code : "UNKNOWN");
    snprintf(e->message, sizeof(e->message), "%s", message ? message : "No message provided.");
    snprintf(e->recovery, sizeof(e->recovery), "%s", recovery ? recovery : "");

    if (!g_head) g_head = e;
    else g_tail->next = e;
    g_tail = e;

    if (sev == ERR_SEV_WARNING) g_warn_count++;
    else if (sev == ERR_SEV_ERROR) g_err_count++;
    else if (sev == ERR_SEV_FATAL) g_fatal_count++;

    fprintf(stderr, "[%s][%s] line=%d col=%d code=%s: %s",
            sev_text(sev), phase_text(phase), line, col, e->code, e->message);
    if (e->recovery[0] != '\0') fprintf(stderr, " | recovery: %s", e->recovery);
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
int error_has_fatal(void) { return g_fatal_count > 0; }
int error_warning_count(void) { return g_warn_count; }
int error_error_count(void) { return g_err_count; }
int error_fatal_count(void) { return g_fatal_count; }

void error_print_all(FILE *out) {
    ErrorEntry *p = g_head;
    while (p) {
        fprintf(out, "[%s][%s] line=%d col=%d code=%s: %s",
                sev_text(p->severity), phase_text(p->phase), p->line, p->col, p->code, p->message);
        if (p->recovery[0] != '\0') fprintf(out, " | recovery: %s", p->recovery);
        fputc('\n', out);
        p = p->next;
    }
}

void error_print_summary(FILE *out) {
    fprintf(out, "\n--- Diagnostics Summary ---\n");
    fprintf(out, "Warnings: %d\nErrors: %d\nFatals: %d\n",
            g_warn_count, g_err_count, g_fatal_count);
}

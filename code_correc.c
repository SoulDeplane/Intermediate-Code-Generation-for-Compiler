#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LEN 500

char source_code[MAX_LINES][MAX_LEN];
int total_lines = 0;

void fix_missing_hash_include(int line_idx);
static void fix_missing_closing_braces_at_eof(void);
static void fix_common_header_extensions(int line_idx);
static void replace_identifier_safe(char *line, const char *from, const char *to);
static void apply_typo_table_to_line(int line_idx);
static void detect_and_fix_missing_semicolons_local(void);
static void fix_unbalanced_parens_at_eol(int line_idx);
static void normalize_smart_quotes(int line_idx);
static void fix_wrong_terminator(int line_idx);
static void fix_format_string_missing_percent(int line_idx);
static void fix_unclosed_string_literal(int line_idx);
static void inject_missing_headers(void);
static int  line_uses_identifier(const char *line, const char *ident);
static int  file_has_include(const char *header_basename);
static void prepend_line(const char *text);
static void inject_default_declaration(const char *name);
static int  identifier_is_declared_anywhere(const char *name);
static void ensure_main_returns(void);
static int  find_main_open_brace_line(void);

void load_source_code(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open source file: %s\n", filename);
        exit(1);
    }
    total_lines = 0;
    while (fgets(source_code[total_lines], MAX_LEN, fp)) {
        total_lines++;
        if (total_lines >= MAX_LINES) {
            fprintf(stderr, "Warning: Source truncated to %d lines.\n", MAX_LINES);
            break;
        }
    }
    fclose(fp);
}

void trim_trailing_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static int starts_with_word(const char *s, const char *w) {
    size_t n = strlen(w);
    if (strncmp(s, w, n) != 0) return 0;
    char c = s[n];
    return c == '\0' || isspace((unsigned char)c) || c == '(';
}

static int is_semicolon_candidate_line(const char *line) {
    if (!line) return 0;
    char buf[MAX_LEN];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_trailing_whitespace(buf);

    char *p = ltrim(buf);
    if (*p == '\0') return 0;

    if (*p == '#') return 0;
    if (starts_with_word(p, "include")) return 0;
    if (p[0] == '/' && (p[1] == '/' || p[1] == '*')) return 0;

    if (strcmp(p, "{") == 0 || strcmp(p, "}") == 0) return 0;

    size_t len = strlen(p);
    char last = p[len - 1];
    if (last == ';' || last == '{' || last == '}' || last == ':') return 0;
    if (len >= 2 && p[len - 1] == '/' && p[len - 2] == '*') return 0;

    if (starts_with_word(p, "if") || starts_with_word(p, "for") || starts_with_word(p, "while") ||
        starts_with_word(p, "switch") || starts_with_word(p, "else") || starts_with_word(p, "do")) {
        return 0;
    }

    return 1;
}

static int is_stmt_header_line(const char *line) {
    if (!line) return 0;
    char buf[MAX_LEN];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *p = ltrim(buf);
    return starts_with_word(p, "if") || starts_with_word(p, "for") || starts_with_word(p, "while") ||
           starts_with_word(p, "switch") || starts_with_word(p, "else") || starts_with_word(p, "do");
}

static void fix_missing_semicolon_inline(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    if (is_stmt_header_line(source_code[line_idx])) return;

    char *line = source_code[line_idx];
    if (!line || !*line) return;

    const char *eq = strchr(line, '=');
    if (!eq) return;

    int in_str = 0, in_chr = 0, esc = 0;
    size_t n = strlen(line);
    size_t i = (size_t)(eq - line) + 1;

    while (i < n && isspace((unsigned char)line[i])) i++;

    for (; i < n; i++) {
        char c = line[i];
        if (esc) { esc = 0; continue; }
        if (c == '\\') { esc = 1; continue; }
        if (!in_chr && c == '"' ) { in_str = !in_str; continue; }
        if (!in_str && c == '\'') { in_chr = !in_chr; continue; }
        if (in_str || in_chr) continue;

        if (isspace((unsigned char)c)) {
            size_t j = i;
            while (j < n && isspace((unsigned char)line[j])) j++;
            if (j < n && (isalpha((unsigned char)line[j]) || line[j] == '_')) {
                size_t k = j;
                while (k < n && (isalnum((unsigned char)line[k]) || line[k] == '_')) k++;
                while (k < n && isspace((unsigned char)line[k])) k++;
                if (k < n && line[k] == '(') {
                    char out[MAX_LEN];
                    size_t out_i = 0;
                    for (size_t x = 0; x < j && out_i + 1 < sizeof(out); x++) out[out_i++] = line[x];
                    if (out_i + 2 < sizeof(out)) { out[out_i++] = ';'; out[out_i++] = ' '; }
                    for (size_t x = j; x < n && out_i + 1 < sizeof(out); x++) out[out_i++] = line[x];
                    out[out_i] = '\0';
                    strncpy(source_code[line_idx], out, MAX_LEN - 1);
                    source_code[line_idx][MAX_LEN - 1] = '\0';
                    return;
                }
            }
        }
    }
}

void fix_missing_semicolon(int line) {
    if (line < 0 || line >= total_lines) return;

    fix_missing_semicolon_inline(line);

    if (!is_semicolon_candidate_line(source_code[line])) {
        for (int j = line - 1; j >= 0; j--) {
            if (is_semicolon_candidate_line(source_code[j])) {
                line = j;
                break;
            }
            char buf[MAX_LEN];
            strncpy(buf, source_code[j], sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            char *p = ltrim(buf);
            trim_trailing_whitespace(p);
            if (*p == '#' || strcmp(p, "{") == 0) break;
        }
        if (!is_semicolon_candidate_line(source_code[line])) return;
    }

    trim_trailing_whitespace(source_code[line]);
    int len = (int)strlen(source_code[line]);
    if (len <= 0) return;

    if (source_code[line][len - 1] != ';') {
        if (source_code[line][len - 1] == ')') {
            for (int k = line + 1; k < total_lines; k++) {
                char buf[MAX_LEN];
                strncpy(buf, source_code[k], sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                trim_trailing_whitespace(buf);
                char *p = ltrim(buf);
                if (*p == '\0') continue;
                if (strcmp(p, "{") == 0) return;
                break;
            }
        }
        strcat(source_code[line], ";\n");
    }
}

static int is_ident_char(int c) {
    return isalnum((unsigned char)c) || c == '_';
}

static void apply_heuristic_fixes() {
    for (int i = 0; i < total_lines; i++) {
        char *ptr = source_code[i];
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr && *ptr != '#' && strncmp(ptr, "include", 7) == 0) {
            fix_missing_hash_include(i);
        }
        fix_common_header_extensions(i);

        normalize_smart_quotes(i);
        fix_unclosed_string_literal(i);

        ptr = source_code[i];
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (starts_with_word(ptr, "main")) {
            if (!(starts_with_word(ptr, "int") || starts_with_word(ptr, "float") ||
                  starts_with_word(ptr, "char") || starts_with_word(ptr, "void"))) {
                if (strncmp(ptr, "main", 4) == 0) {
                    char *after = ptr + 4;
                    while (*after && isspace((unsigned char)*after)) after++;
                    if (*after == '(') {
                        char temp[MAX_LEN];
                        char *orig_ptr = source_code[i];
                        int lead_spaces = (int)(ptr - orig_ptr);
                        if (lead_spaces < 0) lead_spaces = 0;
                        strncpy(temp, orig_ptr, (size_t)lead_spaces);
                        temp[lead_spaces] = '\0';
                        strncat(temp, "int ", sizeof(temp) - strlen(temp) - 1);
                        strncat(temp, ptr, sizeof(temp) - strlen(temp) - 1);
                        strncpy(source_code[i], temp, MAX_LEN - 1);
                        source_code[i][MAX_LEN - 1] = '\0';
                    }
                }
            }
        }

        if (strstr(source_code[i], "man(") || strstr(source_code[i], "man (")) {
            replace_identifier_safe(source_code[i], "man", "main");
        }

        replace_identifier_safe(source_code[i], "print", "printf");

        apply_typo_table_to_line(i);

        fix_format_string_missing_percent(i);
        fix_wrong_terminator(i);

        fix_unbalanced_parens_at_eol(i);
    }

    detect_and_fix_missing_semicolons_local();
}

void save_corrected_code(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Cannot open output file: %s\n", filename);
        exit(1);
    }
    for (int i = 0; i < total_lines; i++) {
        fputs(source_code[i], fp);
        size_t len = strlen(source_code[i]);
        if (len == 0 || source_code[i][len - 1] != '\n') {
            fputs("\n", fp);
        }
    }
    fclose(fp);
}

int fixed_include_lines[MAX_LINES] = {0};

void fix_missing_hash_include(int line_idx) {
    if (line_idx >= 0 && line_idx < total_lines) {
        if (fixed_include_lines[line_idx]) return;
        char *ptr = source_code[line_idx];
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (strncmp(ptr, "include", 7) == 0) {
            char temp[MAX_LEN];
            char *orig_ptr = source_code[line_idx];
            int lead_spaces = ptr - orig_ptr;
            
            strncpy(temp, orig_ptr, lead_spaces);
            temp[lead_spaces] = '#';
            strcpy(temp + lead_spaces + 1, ptr);
            
            strcpy(source_code[line_idx], temp);
            fixed_include_lines[line_idx] = 1;
        }
    }
}

static void fix_common_header_extensions(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line) return;

    static const struct { const char *from; const char *to; } header_fixups[] = {
        {"#include<stdio>",    "#include<stdio.h>"},
        {"#include<stdlib>",   "#include<stdlib.h>"},
        {"#include<string>",   "#include<string.h>"},
        {"#include<math>",     "#include<math.h>"},
        {"#include<ctype>",    "#include<ctype.h>"},
        {"#include<time>",     "#include<time.h>"},
        {"#include<assert>",   "#include<assert.h>"},
        {"#include<errno>",    "#include<errno.h>"},
        {"#include<limits>",   "#include<limits.h>"},
        {"#include<float>",    "#include<float.h>"},
        {"#include<stddef>",   "#include<stddef.h>"},
        {"#include<stdint>",   "#include<stdint.h>"},
        {"#include<stdbool>",  "#include<stdbool.h>"},
        {"#include<stdarg>",   "#include<stdarg.h>"},
        {"#include<signal>",   "#include<signal.h>"},
        {"#include<setjmp>",   "#include<setjmp.h>"},
        {"#include <stdio>",   "#include <stdio.h>"},
        {"#include <stdlib>",  "#include <stdlib.h>"},
        {"#include <string>",  "#include <string.h>"},
        {"#include <math>",    "#include <math.h>"},
        {"#include <ctype>",   "#include <ctype.h>"},
        {"#include <time>",    "#include <time.h>"},
        {"#include <assert>",  "#include <assert.h>"},
        {"#include <errno>",   "#include <errno.h>"},
        {"#include <limits>",  "#include <limits.h>"},
        {"#include <float>",   "#include <float.h>"},
        {"#include <stddef>",  "#include <stddef.h>"},
        {"#include <stdint>",  "#include <stdint.h>"},
        {"#include <stdbool>", "#include <stdbool.h>"},
        {"#include <stdarg>",  "#include <stdarg.h>"},
        {"#include <signal>",  "#include <signal.h>"},
        {"#include <setjmp>",  "#include <setjmp.h>"},
        {NULL, NULL}
    };

    for (int k = 0; header_fixups[k].from != NULL; k++) {
        char *pos = strstr(source_code[line_idx], header_fixups[k].from);
        if (!pos) continue;
        char out[MAX_LEN];
        size_t prefix = (size_t)(pos - source_code[line_idx]);
        snprintf(out, sizeof(out), "%.*s%s%s",
                 (int)prefix, source_code[line_idx],
                 header_fixups[k].to, pos + strlen(header_fixups[k].from));
        strncpy(source_code[line_idx], out, MAX_LEN - 1);
        source_code[line_idx][MAX_LEN - 1] = '\0';
    }
}

static void replace_identifier_safe(char *line, const char *from, const char *to) {
    if (!line || !from || !to) return;
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);
    if (from_len == 0 || from_len > 80 || to_len > 80) return;

    char out[MAX_LEN];
    size_t out_i = 0;
    size_t i = 0;
    size_t n = strlen(line);
    int in_str = 0, in_chr = 0, esc = 0;
    int in_line_comment = 0, in_block_comment = 0;

    while (i < n && out_i + 1 < sizeof(out)) {
        char c = line[i];
        if (in_line_comment) { out[out_i++] = c; i++; continue; }
        if (in_block_comment) {
            out[out_i++] = c;
            if (c == '*' && i + 1 < n && line[i+1] == '/') {
                if (out_i + 1 < sizeof(out)) out[out_i++] = '/';
                i += 2;
                in_block_comment = 0;
                continue;
            }
            i++;
            continue;
        }
        if (esc) { out[out_i++] = c; i++; esc = 0; continue; }
        if ((in_str || in_chr) && c == '\\') { out[out_i++] = c; i++; esc = 1; continue; }
        if (!in_chr && c == '"')  { in_str = !in_str; out[out_i++] = c; i++; continue; }
        if (!in_str && c == '\'') { in_chr = !in_chr; out[out_i++] = c; i++; continue; }
        if (in_str || in_chr)     { out[out_i++] = c; i++; continue; }
        if (c == '/' && i + 1 < n && line[i+1] == '/') {
            in_line_comment = 1;
            out[out_i++] = c;
            i++;
            continue;
        }
        if (c == '/' && i + 1 < n && line[i+1] == '*') {
            in_block_comment = 1;
            out[out_i++] = c;
            i++;
            continue;
        }

        if (i + from_len <= n && strncmp(line + i, from, from_len) == 0) {
            int before_ok = (i == 0) || !is_ident_char((unsigned char)line[i - 1]);
            int after_ok  = (i + from_len >= n) || !is_ident_char((unsigned char)line[i + from_len]);
            if (before_ok && after_ok) {
                for (size_t k = 0; k < to_len && out_i + 1 < sizeof(out); k++) out[out_i++] = to[k];
                i += from_len;
                continue;
            }
        }
        out[out_i++] = c;
        i++;
    }
    out[out_i] = '\0';
    strncpy(line, out, MAX_LEN - 1);
    line[MAX_LEN - 1] = '\0';
}

static void apply_typo_table_to_line(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    static const struct { const char *from; const char *to; } typo_table[] = {
        {"itn", "int"}, {"nit", "int"}, {"inta", "int"},
        {"flot", "float"}, {"flaot", "float"}, {"foat", "float"}, {"flaot", "float"},
        {"doulbe", "double"}, {"doble", "double"}, {"duoble", "double"},
        {"chra", "char"}, {"cahr", "char"}, {"chr", "char"},
        {"voi", "void"}, {"vod", "void"}, {"viod", "void"}, {"vid", "void"},
        {"retrun", "return"}, {"reutrn", "return"}, {"retun", "return"},
        {"ruturn", "return"}, {"retrn", "return"}, {"retunr", "return"},
        {"whlie", "while"}, {"whiel", "while"}, {"wile", "while"}, {"whle", "while"},
        {"fro", "for"}, {"ofr", "for"},
        {"esle", "else"}, {"eles", "else"}, {"els", "else"},
        {"swich", "switch"}, {"swithc", "switch"}, {"siwtch", "switch"},
        {"brek", "break"}, {"braek", "break"}, {"breka", "break"},
        {"contineu", "continue"}, {"continu", "continue"}, {"contiue", "continue"},
        {"caes", "case"}, {"csae", "case"},
        {"defualt", "default"}, {"deafult", "default"}, {"defalt", "default"},
        {"goot", "goto"}, {"gooto", "goto"},
        {"strcut", "struct"}, {"sturct", "struct"}, {"strut", "struct"},
        {"unoin", "union"}, {"uinon", "union"},
        {"typdef", "typedef"}, {"tyepdef", "typedef"}, {"typedf", "typedef"},
        {"sieof", "sizeof"}, {"sizof", "sizeof"}, {"siezof", "sizeof"},
        {"cosnt", "const"}, {"conts", "const"}, {"cnst", "const"},
        {"statci", "static"}, {"staic", "static"}, {"sttaic", "static"},
        {"extren", "extern"}, {"exetrn", "extern"},
        {"emun", "enum"}, {"enmu", "enum"},
        {"shrot", "short"}, {"shotr", "short"},
        {"singed", "signed"}, {"sigend", "signed"},
        {"unsinged", "unsigned"}, {"unisgned", "unsigned"},
        {"pintf", "printf"}, {"prinft", "printf"}, {"printff", "printf"},
        {"prnitf", "printf"}, {"primtf", "printf"}, {"prinf", "printf"},
        {"pirntf", "printf"}, {"pirnt", "printf"},
        {"scnaf", "scanf"}, {"scnf", "scanf"}, {"scanff", "scanf"},
        {"sacnf", "scanf"}, {"scnaff", "scanf"}, {"scfan", "scanf"},
        {"fpintf", "fprintf"}, {"fprintff", "fprintf"},
        {"fscnaf", "fscanf"}, {"fscanff", "fscanf"},
        {"spintf", "sprintf"}, {"sprintff", "sprintf"},
        {"sscnaf", "sscanf"}, {"sscanff", "sscanf"},
        {"foepn", "fopen"}, {"foppen", "fopen"}, {"fopn", "fopen"},
        {"fcose", "fclose"}, {"fclsoe", "fclose"}, {"flcose", "fclose"},
        {"fgest", "fgets"}, {"fgetss", "fgets"}, {"fgts", "fgets"},
        {"fpust", "fputs"}, {"fputss", "fputs"},
        {"getss", "gets"}, {"putts", "puts"},
        {"getcahr", "getchar"}, {"getchr", "getchar"},
        {"putcahr", "putchar"}, {"putchr", "putchar"},
        {"mallo", "malloc"}, {"malooc", "malloc"}, {"mallock", "malloc"}, {"mallcoc", "malloc"},
        {"callco", "calloc"}, {"callcoc", "calloc"}, {"clloc", "calloc"},
        {"relaloc", "realloc"}, {"raelloc", "realloc"}, {"reaaloc", "realloc"},
        {"fre", "free"}, {"freee", "free"},
        {"exti", "exit"}, {"exitt", "exit"},
        {"atio", "atoi"}, {"atfo", "atof"},
        {"strln", "strlen"}, {"srtlen", "strlen"}, {"strlne", "strlen"},
        {"srtcpy", "strcpy"}, {"strcyp", "strcpy"}, {"strcpyy", "strcpy"},
        {"srtncpy", "strncpy"}, {"strncyp", "strncpy"},
        {"srtcat", "strcat"}, {"strcatt", "strcat"}, {"strcta", "strcat"},
        {"srtncat", "strncat"}, {"strncta", "strncat"},
        {"srtcmp", "strcmp"}, {"strcmpp", "strcmp"}, {"strmcp", "strcmp"},
        {"srtncmp", "strncmp"}, {"strncmpp", "strncmp"},
        {"strchar", "strchr"}, {"srtchr", "strchr"},
        {"srtstr", "strstr"}, {"strsrt", "strstr"},
        {"memcyp", "memcpy"}, {"memcpyy", "memcpy"}, {"memcyy", "memcpy"},
        {"memste", "memset"}, {"memsett", "memset"}, {"memsst", "memset"},
        {"isaplha", "isalpha"}, {"isalpa", "isalpha"},
        {"isdigt", "isdigit"}, {"isidigit", "isdigit"},
        {"isspce", "isspace"}, {"issapce", "isspace"},
        {"isalnumm", "isalnum"}, {"isaplnum", "isalnum"},
        {"toupepr", "toupper"}, {"toupr", "toupper"},
        {"tolwoer", "tolower"}, {"tolwer", "tolower"},
        {"powr", "pow"}, {"poww", "pow"},
        {"sqrtt", "sqrt"}, {"sqart", "sqrt"}, {"sqret", "sqrt"},
        {"fabss", "fabs"}, {"fbas", "fabs"},
        {"indlue", "include"}, {"inculde", "include"}, {"incldue", "include"},
        {"inlcude", "include"}, {"iclude", "include"}, {"incude", "include"},
        {"icnlude", "include"}, {"inclue", "include"}, {"includ", "include"},
        {"imclude", "include"}, {"inclde", "include"}, {"ncldue", "include"},
        {NULL, NULL}
    };
    for (int k = 0; typo_table[k].from != NULL; k++) {
        replace_identifier_safe(source_code[line_idx], typo_table[k].from, typo_table[k].to);
    }
}

static int next_nonblank_starts_with_brace(int i) {
    for (int j = i + 1; j < total_lines; j++) {
        char buf[MAX_LEN];
        strncpy(buf, source_code[j], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char *p = ltrim(buf);
        trim_trailing_whitespace(p);
        if (*p == '\0') continue;
        return *p == '{';
    }
    return 0;
}

static void detect_and_fix_missing_semicolons_local(void) {
    for (int i = 0; i < total_lines; i++) {
        if (!is_semicolon_candidate_line(source_code[i])) continue;
        trim_trailing_whitespace(source_code[i]);
        size_t len = strlen(source_code[i]);
        if (len == 0 || source_code[i][len - 1] == ';') continue;
        if (source_code[i][len - 1] == ')' && next_nonblank_starts_with_brace(i)) continue;
        if (len + 2 < MAX_LEN) {
            strcat(source_code[i], ";\n");
        }
    }
}

static void fix_unbalanced_parens_at_eol(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line || !*line) return;

    int paren = 0, brack = 0;
    int in_str = 0, in_chr = 0, esc = 0;
    int has_call = 0, has_index = 0;
    size_t n = strlen(line);

    for (size_t i = 0; i < n; i++) {
        char c = line[i];
        if (esc) { esc = 0; continue; }
        if ((in_str || in_chr) && c == '\\') { esc = 1; continue; }
        if (!in_chr && c == '"')  { in_str = !in_str; continue; }
        if (!in_str && c == '\'') { in_chr = !in_chr; continue; }
        if (in_str || in_chr) continue;

        if (c == '(') {
            paren++;
            size_t j = i;
            while (j > 0 && isspace((unsigned char)line[j - 1])) j--;
            if (j > 0 && (isalnum((unsigned char)line[j - 1]) || line[j - 1] == '_')) {
                has_call = 1;
            }
        } else if (c == ')') {
            paren--;
        } else if (c == '[') {
            brack++;
            size_t j = i;
            while (j > 0 && isspace((unsigned char)line[j - 1])) j--;
            if (j > 0 && (isalnum((unsigned char)line[j - 1]) || line[j - 1] == '_' ||
                          line[j - 1] == ')' || line[j - 1] == ']')) {
                has_index = 1;
            }
        } else if (c == ']') {
            brack--;
        }
    }

    if (paren < 0 || brack < 0) return;
    if (paren > 1 || brack > 1) return;
    if (paren == 0 && brack == 0) return;
    if (paren == 1 && !has_call)  return;
    if (brack == 1 && !has_index) return;

    size_t end = n;
    while (end > 0 && (line[end - 1] == '\n' || line[end - 1] == '\r')) end--;
    int had_semi = 0;
    size_t insert_pos = end;
    while (insert_pos > 0 && isspace((unsigned char)line[insert_pos - 1])) insert_pos--;
    if (insert_pos > 0 && line[insert_pos - 1] == ';') {
        had_semi = 1;
        insert_pos--;
        while (insert_pos > 0 && isspace((unsigned char)line[insert_pos - 1])) insert_pos--;
    }

    char buf[MAX_LEN];
    size_t bi = 0;
    for (size_t i = 0; i < insert_pos && bi + 4 < sizeof(buf); i++) buf[bi++] = line[i];
    if (paren == 1 && bi + 4 < sizeof(buf)) buf[bi++] = ')';
    if (brack == 1 && bi + 4 < sizeof(buf)) buf[bi++] = ']';
    if (had_semi && bi + 2 < sizeof(buf)) buf[bi++] = ';';
    if (bi + 1 < sizeof(buf)) buf[bi++] = '\n';
    buf[bi] = '\0';

    strncpy(source_code[line_idx], buf, MAX_LEN - 1);
    source_code[line_idx][MAX_LEN - 1] = '\0';
}

static void normalize_smart_quotes(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line) return;
    size_t n = strlen(line);
    char out[MAX_LEN];
    size_t oi = 0;
    for (size_t i = 0; i < n && oi + 1 < sizeof(out); ) {
        unsigned char b0 = (unsigned char)line[i];
        if (b0 == 0xE2 && i + 2 < n) {
            unsigned char b1 = (unsigned char)line[i+1];
            unsigned char b2 = (unsigned char)line[i+2];
            if (b1 == 0x80 && (b2 == 0x9C || b2 == 0x9D)) { out[oi++] = '"';  i += 3; continue; }
            if (b1 == 0x80 && (b2 == 0x98 || b2 == 0x99)) { out[oi++] = '\''; i += 3; continue; }
        }
        out[oi++] = line[i++];
    }
    out[oi] = '\0';
    strncpy(source_code[line_idx], out, MAX_LEN - 1);
    source_code[line_idx][MAX_LEN - 1] = '\0';
}

static void fix_wrong_terminator(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line || !*line) return;

    char buf[MAX_LEN];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_trailing_whitespace(buf);
    char *p = ltrim(buf);
    if (*p == '\0' || *p == '#') return;

    size_t pl = strlen(p);
    if (pl < 1) return;
    char last = p[pl - 1];
    if (last != ',' && last != ':') return;

    if (last == ':') {
        if (starts_with_word(p, "case") || starts_with_word(p, "default")) return;
        int has_ws = 0;
        for (size_t i = 0; i + 1 < pl; i++) {
            if (isspace((unsigned char)p[i])) { has_ws = 1; break; }
            if (!is_ident_char((unsigned char)p[i])) return;
        }
        if (!has_ws) return;
    }

    int in_str = 0, in_chr = 0, esc = 0;
    int saw_brace = 0;
    for (size_t i = 0; i < pl; i++) {
        char c = p[i];
        if (esc) { esc = 0; continue; }
        if ((in_str || in_chr) && c == '\\') { esc = 1; continue; }
        if (!in_chr && c == '"')  { in_str = !in_str; continue; }
        if (!in_str && c == '\'') { in_chr = !in_chr; continue; }
        if (in_str || in_chr) continue;
        if (c == '{' || c == '}') saw_brace = 1;
    }
    if (saw_brace) return;

    size_t n = strlen(line);
    size_t end = n;
    while (end > 0 && (line[end-1] == '\n' || line[end-1] == '\r')) end--;
    while (end > 0 && isspace((unsigned char)line[end-1])) end--;
    if (end == 0) return;
    if (line[end-1] != ',' && line[end-1] != ':') return;

    char out[MAX_LEN];
    size_t oi = 0;
    for (size_t i = 0; i < end - 1 && oi + 3 < sizeof(out); i++) out[oi++] = line[i];
    if (oi + 2 < sizeof(out)) out[oi++] = ';';
    if (oi + 1 < sizeof(out)) out[oi++] = '\n';
    out[oi] = '\0';
    strncpy(source_code[line_idx], out, MAX_LEN - 1);
    source_code[line_idx][MAX_LEN - 1] = '\0';
}

static int looks_like_format_letter(const char *s, size_t n, size_t i, size_t *adv_out) {
    if (i >= n) return 0;
    char c = s[i];
    if (i + 1 < n && (c == 'l' || c == 'h')) {
        char d = s[i+1];
        if (i + 2 < n && c == 'l' && d == 'l') {
            char e = s[i+2];
            if (e == 'd' || e == 'i' || e == 'u' || e == 'x' || e == 'o') {
                *adv_out = 3; return 1;
            }
        }
        if (d == 'd' || d == 'i' || d == 'u' || d == 'x' || d == 'o' ||
            d == 'f' || d == 'e' || d == 'g') {
            *adv_out = 2; return 1;
        }
    }
    if (c == 'd' || c == 'i' || c == 'u' || c == 'x' || c == 'o' ||
        c == 'f' || c == 'e' || c == 'g' || c == 's' || c == 'c' || c == 'p') {
        *adv_out = 1; return 1;
    }
    return 0;
}

static void fix_format_string_missing_percent(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line || !*line) return;

    static const char *fmt_funcs[] = {
        "printf", "scanf", "fprintf", "fscanf", "sprintf", "sscanf",
        "snprintf", "puts", NULL
    };
    int has_fmt_call = 0;
    for (int k = 0; fmt_funcs[k]; k++) {
        const char *f = fmt_funcs[k];
        size_t fl = strlen(f);
        const char *pos = line;
        while ((pos = strstr(pos, f)) != NULL) {
            int before_ok = (pos == line) || !is_ident_char((unsigned char)*(pos - 1));
            const char *after = pos + fl;
            while (*after == ' ' || *after == '\t') after++;
            if (before_ok && *after == '(') { has_fmt_call = 1; break; }
            pos = pos + fl;
        }
        if (has_fmt_call) break;
    }
    if (!has_fmt_call) return;

    size_t n = strlen(line);
    char out[MAX_LEN];
    size_t oi = 0;
    int in_str = 0, in_chr = 0, esc = 0;
    char prev_in_str = 0;
    size_t i = 0;
    while (i < n && oi + 1 < sizeof(out)) {
        char c = line[i];
        if (esc) { out[oi++] = c; i++; esc = 0; prev_in_str = c; continue; }
        if ((in_str || in_chr) && c == '\\') { out[oi++] = c; i++; esc = 1; prev_in_str = c; continue; }
        if (!in_chr && c == '"')  {
            int opening = !in_str;
            in_str = !in_str;
            out[oi++] = c; i++;
            if (opening) prev_in_str = '"';
            continue;
        }
        if (!in_str && c == '\'') { in_chr = !in_chr; out[oi++] = c; i++; continue; }

        if (in_str) {
            size_t adv = 0;
            int prev_ok = (prev_in_str == '"' || prev_in_str == ' ' ||
                           prev_in_str == ':' || prev_in_str == '=' ||
                           prev_in_str == ',' || prev_in_str == '\t');
            if (prev_ok && looks_like_format_letter(line, n, i, &adv)) {
                size_t after = i + adv;
                int follow_ok = 0;
                if (after < n) {
                    if (line[after] == '"') follow_ok = 1;
                    else if (line[after] == '\\' && after + 1 < n &&
                             (line[after+1] == 'n' || line[after+1] == 't')) follow_ok = 1;
                    else if (line[after] == ' ' || line[after] == '\t') follow_ok = 1;
                }
                if (follow_ok && oi + 1 + adv < sizeof(out)) {
                    out[oi++] = '%';
                    for (size_t k = 0; k < adv; k++) out[oi++] = line[i + k];
                    i += adv;
                    prev_in_str = line[i - 1];
                    continue;
                }
            }
            prev_in_str = c;
        }
        out[oi++] = c;
        i++;
    }
    out[oi] = '\0';
    strncpy(source_code[line_idx], out, MAX_LEN - 1);
    source_code[line_idx][MAX_LEN - 1] = '\0';
}

static void fix_unclosed_string_literal(int line_idx) {
    if (line_idx < 0 || line_idx >= total_lines) return;
    char *line = source_code[line_idx];
    if (!line || !*line) return;
    if (strstr(line, "//") || strstr(line, "/*")) return;

    size_t n = strlen(line);
    int dq = 0, in_chr = 0, esc = 0;
    for (size_t i = 0; i < n; i++) {
        char c = line[i];
        if (esc) { esc = 0; continue; }
        if (c == '\\') { esc = 1; continue; }
        if (!in_chr && c == '"') dq++;
        else if (c == '\'' && (dq % 2 == 0)) in_chr = !in_chr;
    }
    if (dq % 2 == 0) return;

    size_t end = n;
    while (end > 0 && (line[end-1] == '\n' || line[end-1] == '\r')) end--;
    int had_semi = 0;
    size_t insert_pos = end;
    while (insert_pos > 0 && isspace((unsigned char)line[insert_pos-1])) insert_pos--;
    if (insert_pos > 0 && line[insert_pos-1] == ';') {
        had_semi = 1;
        insert_pos--;
        while (insert_pos > 0 && isspace((unsigned char)line[insert_pos-1])) insert_pos--;
    }

    char buf[MAX_LEN];
    size_t bi = 0;
    for (size_t i = 0; i < insert_pos && bi + 4 < sizeof(buf); i++) buf[bi++] = line[i];
    if (bi + 4 < sizeof(buf)) buf[bi++] = '"';
    if (had_semi && bi + 2 < sizeof(buf)) buf[bi++] = ';';
    if (bi + 1 < sizeof(buf)) buf[bi++] = '\n';
    buf[bi] = '\0';
    strncpy(source_code[line_idx], buf, MAX_LEN - 1);
    source_code[line_idx][MAX_LEN - 1] = '\0';
}

static int line_uses_identifier(const char *line, const char *ident) {
    if (!line || !ident || !*ident) return 0;
    size_t il = strlen(ident);
    size_t n = strlen(line);
    int in_str = 0, in_chr = 0, esc = 0;
    int in_line_comment = 0, in_block_comment = 0;
    for (size_t i = 0; i < n; i++) {
        char c = line[i];
        if (in_line_comment) continue;
        if (in_block_comment) {
            if (c == '*' && i + 1 < n && line[i+1] == '/') { in_block_comment = 0; i++; }
            continue;
        }
        if (esc) { esc = 0; continue; }
        if ((in_str || in_chr) && c == '\\') { esc = 1; continue; }
        if (!in_chr && c == '"')  { in_str = !in_str; continue; }
        if (!in_str && c == '\'') { in_chr = !in_chr; continue; }
        if (in_str || in_chr) continue;
        if (c == '/' && i + 1 < n && line[i+1] == '/') { in_line_comment = 1; continue; }
        if (c == '/' && i + 1 < n && line[i+1] == '*') { in_block_comment = 1; i++; continue; }

        if (i + il <= n && strncmp(line + i, ident, il) == 0) {
            int before_ok = (i == 0) || !is_ident_char((unsigned char)line[i-1]);
            int after_ok  = (i + il >= n) || !is_ident_char((unsigned char)line[i+il]);
            if (before_ok && after_ok) return 1;
        }
    }
    return 0;
}

static int file_has_include(const char *header_basename) {
    if (!header_basename) return 0;
    for (int i = 0; i < total_lines; i++) {
        const char *p = source_code[i];
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p != '#') continue;
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
        if (strncmp(p, "include", 7) != 0) continue;
        if (strstr(source_code[i], header_basename)) return 1;
    }
    return 0;
}

static void prepend_line(const char *text) {
    if (total_lines >= MAX_LINES) return;
    for (int i = total_lines; i > 0; i--) {
        memcpy(source_code[i], source_code[i-1], MAX_LEN);
    }
    strncpy(source_code[0], text, MAX_LEN - 1);
    source_code[0][MAX_LEN - 1] = '\0';
    total_lines++;
}

static void inject_missing_headers(void) {
    static const struct {
        const char *header;
        const char *include_line;
        const char *funcs[24];
    } header_map[] = {
        {"stdio.h",  "#include <stdio.h>\n",
         {"printf","scanf","fprintf","fscanf","sprintf","sscanf","snprintf",
          "fopen","fclose","fread","fwrite","fgets","fputs","getchar","putchar",
          "gets","puts","perror", NULL}},
        {"stdlib.h", "#include <stdlib.h>\n",
         {"malloc","calloc","realloc","free","exit","atoi","atof","atol",
          "abs","rand","srand","qsort","system", NULL}},
        {"string.h", "#include <string.h>\n",
         {"strlen","strcpy","strncpy","strcat","strncat","strcmp","strncmp",
          "strchr","strrchr","strstr","memcpy","memmove","memset","memcmp","strdup", NULL}},
        {"ctype.h",  "#include <ctype.h>\n",
         {"isalpha","isdigit","isalnum","isspace","isupper","islower","ispunct",
          "toupper","tolower", NULL}},
        {"math.h",   "#include <math.h>\n",
         {"pow","sqrt","sin","cos","tan","log","log10","exp","fabs","floor","ceil","round", NULL}},
        {"time.h",   "#include <time.h>\n",
         {"clock","difftime","mktime","localtime","strftime", NULL}},
        {NULL, NULL, {NULL}}
    };

    for (int h = 0; header_map[h].header != NULL; h++) {
        if (file_has_include(header_map[h].header)) continue;
        int needed = 0;
        for (int f = 0; header_map[h].funcs[f] != NULL && !needed; f++) {
            const char *fn = header_map[h].funcs[f];
            for (int i = 0; i < total_lines; i++) {
                if (line_uses_identifier(source_code[i], fn)) {
                    char *p = source_code[i];
                    while (*p && isspace((unsigned char)*p)) p++;
                    if (*p == '#') continue;
                    needed = 1;
                    break;
                }
            }
        }
        if (needed) prepend_line(header_map[h].include_line);
    }
}

static int identifier_is_declared_anywhere(const char *name) {
    size_t nl = strlen(name);
    for (int i = 0; i < total_lines; i++) {
        char buf[MAX_LEN];
        strncpy(buf, source_code[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char *p = ltrim(buf);
        if (!(starts_with_word(p, "int") || starts_with_word(p, "float") ||
              starts_with_word(p, "char") || starts_with_word(p, "double") ||
              starts_with_word(p, "long") || starts_with_word(p, "short") ||
              starts_with_word(p, "unsigned") || starts_with_word(p, "signed") ||
              starts_with_word(p, "void"))) {
            continue;
        }
        while (*p && !isspace((unsigned char)*p)) p++;
        while (*p && isspace((unsigned char)*p)) p++;
        while (*p) {
            while (*p == '*' || isspace((unsigned char)*p)) p++;
            if (!isalpha((unsigned char)*p) && *p != '_') break;
            char *id_start = p;
            while (is_ident_char((unsigned char)*p)) p++;
            size_t idl = (size_t)(p - id_start);
            if (idl == nl && strncmp(id_start, name, nl) == 0) return 1;
            while (*p && *p != ',' && *p != ';') p++;
            if (*p == ',') p++;
            else break;
        }
    }
    return 0;
}

static void inject_default_declaration(const char *name) {
    if (!name || !*name) return;
    int used = 0;
    for (int i = 0; i < total_lines; i++) {
        if (line_uses_identifier(source_code[i], name)) { used = 1; break; }
    }
    if (!used) return;
    if (identifier_is_declared_anywhere(name)) return;

    int main_brace = find_main_open_brace_line();
    int insert_at;
    const char *indent;
    if (main_brace >= 0) {
        insert_at = main_brace + 1;
        indent = "    ";
    } else {
        insert_at = 0;
        for (int i = 0; i < total_lines; i++) {
            char *p = source_code[i];
            while (*p && isspace((unsigned char)*p)) p++;
            if (*p == '#') insert_at = i + 1;
        }
        indent = "";
    }

    if (total_lines >= MAX_LINES) return;
    char text[MAX_LEN];
    snprintf(text, sizeof(text), "%sint %s = 0;\n", indent, name);
    for (int i = total_lines; i > insert_at; i--) {
        memcpy(source_code[i], source_code[i - 1], MAX_LEN);
    }
    strncpy(source_code[insert_at], text, MAX_LEN - 1);
    source_code[insert_at][MAX_LEN - 1] = '\0';
    total_lines++;
}

static int find_main_signature_line(void) {
    for (int i = 0; i < total_lines; i++) {
        char buf[MAX_LEN];
        strncpy(buf, source_code[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char *p = ltrim(buf);
        if (!(starts_with_word(p, "int") || starts_with_word(p, "void"))) continue;
        if (!strstr(source_code[i], "main")) continue;
        return i;
    }
    return -1;
}

static int find_main_open_brace_line(void) {
    int sig = find_main_signature_line();
    if (sig < 0) return -1;
    if (strchr(source_code[sig], '{')) return sig;
    for (int j = sig + 1; j < total_lines; j++) {
        char b2[MAX_LEN];
        strncpy(b2, source_code[j], sizeof(b2) - 1);
        b2[sizeof(b2) - 1] = '\0';
        char *q = ltrim(b2);
        trim_trailing_whitespace(q);
        if (*q == '\0') continue;
        if (*q == '{') return j;
        break;
    }
    return -1;
}

static int main_returns_void(void) {
    int sig = find_main_signature_line();
    if (sig < 0) return 0;
    char buf[MAX_LEN];
    strncpy(buf, source_code[sig], sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *p = ltrim(buf);
    return starts_with_word(p, "void");
}

static void ensure_main_returns(void) {
    if (main_returns_void()) return;
    int open_line = find_main_open_brace_line();
    if (open_line < 0) return;

    int depth = 0;
    int close_line = -1;
    for (int i = open_line; i < total_lines; i++) {
        const char *p = source_code[i];
        int in_str = 0, in_chr = 0, esc = 0;
        while (*p) {
            char c = *p;
            if (esc) { esc = 0; p++; continue; }
            if ((in_str || in_chr) && c == '\\') { esc = 1; p++; continue; }
            if (!in_chr && c == '"')  { in_str = !in_str; p++; continue; }
            if (!in_str && c == '\'') { in_chr = !in_chr; p++; continue; }
            if (!in_str && !in_chr) {
                if (c == '{') depth++;
                else if (c == '}') {
                    depth--;
                    if (depth == 0) { close_line = i; goto found; }
                }
            }
            p++;
        }
    }
found:
    if (close_line < 0) return;

    for (int i = close_line - 1; i > open_line; i--) {
        char buf[MAX_LEN];
        strncpy(buf, source_code[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char *p = ltrim(buf);
        trim_trailing_whitespace(p);
        if (*p == '\0') continue;
        if (starts_with_word(p, "return")) return;
        break;
    }

    if (total_lines >= MAX_LINES) return;
    for (int i = total_lines; i > close_line; i--) {
        memcpy(source_code[i], source_code[i - 1], MAX_LEN);
    }
    strncpy(source_code[close_line], "    return 0;\n", MAX_LEN - 1);
    source_code[close_line][MAX_LEN - 1] = '\0';
    total_lines++;
}

static void fix_missing_closing_braces_at_eof(void) {
    int open = 0, close = 0;
    for (int i = 0; i < total_lines; i++) {
        const char *p = source_code[i];
        while (*p) {
            if (*p == '{') open++;
            else if (*p == '}') close++;
            p++;
        }
    }

    int missing = open - close;
    if (missing <= 0) return;

    while (missing-- > 0 && total_lines < MAX_LINES) {
        strncpy(source_code[total_lines], "}\n", MAX_LEN - 1);
        source_code[total_lines][MAX_LEN - 1] = '\0';
        total_lines++;
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: %s <source.c> <errors.txt> <corrected.c>\n", argv[0]);
        return 1;
    }

    const char *source_file = argv[1];
    const char *errors_file = argv[2];
    const char *output_file = argv[3];

    load_source_code(source_file);
    apply_heuristic_fixes();

    FILE *efp = fopen(errors_file, "r");
    if (efp) {
        char err_type[50];
        int line;
        char symbol[50];

        while (fscanf(efp, "%s", err_type) != EOF) {
            if (strcmp(err_type, "MISSING_SEMICOLON") == 0) {
                if (fscanf(efp, "%d", &line) == 1) {
                    if (line > 0 && line <= total_lines) {
                        fix_missing_semicolon(line - 1);
                    } else {
                        fprintf(stderr, "Warning: Ignoring invalid line number %d for MISSING_SEMICOLON.\n", line);
                    }
                } else {
                    fprintf(stderr, "Warning: Malformed MISSING_SEMICOLON record in %s.\n", errors_file);
                    break;
                }
            } else if (strcmp(err_type, "UNDECLARED_VARIABLE") == 0) {
                if (fscanf(efp, "%d %49s", &line, symbol) != 2) {
                    fprintf(stderr, "Warning: Malformed UNDECLARED_VARIABLE record in %s.\n", errors_file);
                    break;
                }
                inject_default_declaration(symbol);
            } else if (strcmp(err_type, "UNUSED_VARIABLE") == 0) {
                if (fscanf(efp, "%d %49s", &line, symbol) != 2) {
                    fprintf(stderr, "Warning: Malformed UNUSED_VARIABLE record in %s.\n", errors_file);
                    break;
                }
            } else {
                fprintf(stderr, "Warning: Unknown error token '%s' in %s.\n", err_type, errors_file);
            }
        }
        fclose(efp);
    } else {
        fprintf(stderr, "Warning: Cannot open errors file %s, applying formatting only.\n", errors_file);
    }

    fix_missing_closing_braces_at_eof();
    ensure_main_returns();
    inject_missing_headers();

    save_corrected_code(output_file);
    printf("Corrected code saved to %s\n", output_file);

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LEN 500

typedef enum {
    ERR_MISSING_SEMICOLON,
    ERR_UNUSED_VARIABLE,
    ERR_UNDECLARED_VARIABLE
} ErrorType;

typedef struct {
    ErrorType type;
    int line;
    char symbol[50];
} CompilerError;

char source_code[MAX_LINES][MAX_LEN];
int total_lines = 0;

void fix_missing_hash_include(int line_idx);
static void fix_missing_closing_braces_at_eof(void);
static void fix_common_header_extensions(int line_idx);

void load_source_code(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Cannot open source file: %s\n", filename);
        exit(1);
    }
    while (fgets(source_code[total_lines], MAX_LEN, fp)) {
        total_lines++;
        if (total_lines >= MAX_LINES) break;
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

    if (strcmp(p, "{") == 0 || strcmp(p, "}") == 0) return 0;

    size_t len = strlen(p);
    char last = p[len - 1];
    if (last == ';' || last == '{' || last == '}' || last == ':') return 0;

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

void remove_unused_variable(const char *symbol) {

    for (int i = 0; i < total_lines; i++) {
        char *pos = strstr(source_code[i], symbol);
        if (pos) {

            int is_start = (pos == source_code[i] || (!isalnum(*(pos - 1)) && *(pos - 1) != '_'));
            int is_end = (!isalnum(*(pos + strlen(symbol))) && *(pos + strlen(symbol)) != '_');
            if (is_start && is_end) {

                char temp[MAX_LEN];
                snprintf(temp, sizeof(temp), " ");
                strncpy(source_code[i], temp, MAX_LEN - 1);
            }
        }
    }
}

static int is_ident_char(int c) {
    return isalnum((unsigned char)c) || c == '_';
}

static void replace_identifier_token_in_line(char *line, const char *from, const char *to) {
    if (!line || !from || !to) return;
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);
    if (from_len == 0 || from_len > 80 || to_len > 80) return;

    char out[MAX_LEN];
    size_t out_i = 0;
    size_t i = 0;
    size_t n = strlen(line);

    while (i < n && out_i + 1 < sizeof(out)) {
        char *pos = strstr(line + i, from);
        if (!pos) {
            while (i < n && out_i + 1 < sizeof(out)) out[out_i++] = line[i++];
            break;
        }

        size_t pos_i = (size_t)(pos - line);
        while (i < pos_i && out_i + 1 < sizeof(out)) out[out_i++] = line[i++];

        int before_ok = (pos_i == 0) || !is_ident_char((unsigned char)line[pos_i - 1]);
        int after_ok = (pos_i + from_len >= n) || !is_ident_char((unsigned char)line[pos_i + from_len]);

        if (before_ok && after_ok) {
            for (size_t k = 0; k < to_len && out_i + 1 < sizeof(out); k++) out[out_i++] = to[k];
            i = pos_i + from_len;
        } else {
            if (out_i + 1 < sizeof(out)) out[out_i++] = line[i++];
        }
    }

    out[out_i] = '\0';
    strncpy(line, out, MAX_LEN - 1);
    line[MAX_LEN - 1] = '\0';
}

static void apply_heuristic_fixes() {
    for (int i = 0; i < total_lines; i++) {
        char *ptr = source_code[i];
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr && *ptr != '#' && strncmp(ptr, "include", 7) == 0) {
            fix_missing_hash_include(i);
        }
        fix_common_header_extensions(i);

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
            replace_identifier_token_in_line(source_code[i], "man", "main");
        }

        replace_identifier_token_in_line(source_code[i], "print", "printf");
    }
}

void save_corrected_code(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Cannot open output file: %s\n", filename);
        exit(1);
    }
    for (int i = 0; i < total_lines; i++) {
        fputs(source_code[i], fp);

        if (source_code[i][strlen(source_code[i]) - 1] != '\n') {
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

    if (strstr(line, "#include<stdio>")) {
        char out[MAX_LEN];
        size_t n = strlen(line);
        const char *from = "#include<stdio>";
        const char *to = "#include<stdio.h>";
        char *pos = strstr(line, from);
        if (pos) {
            size_t prefix = (size_t)(pos - line);
            snprintf(out, sizeof(out), "%.*s%s%s",
                     (int)prefix, line, to, pos + strlen(from));
            strncpy(source_code[line_idx], out, MAX_LEN - 1);
            source_code[line_idx][MAX_LEN - 1] = '\0';
        }
        (void)n;
    }
    if (strstr(line, "#include<stdlib>")) {
        char out[MAX_LEN];
        const char *from = "#include<stdlib>";
        const char *to = "#include<stdlib.h>";
        char *pos = strstr(line, from);
        if (pos) {
            size_t prefix = (size_t)(pos - line);
            snprintf(out, sizeof(out), "%.*s%s%s",
                     (int)prefix, line, to, pos + strlen(from));
            strncpy(source_code[line_idx], out, MAX_LEN - 1);
            source_code[line_idx][MAX_LEN - 1] = '\0';
        }
    }
    if (strstr(line, "#include<string>")) {
        char out[MAX_LEN];
        const char *from = "#include<string>";
        const char *to = "#include<string.h>";
        char *pos = strstr(line, from);
        if (pos) {
            size_t prefix = (size_t)(pos - line);
            snprintf(out, sizeof(out), "%.*s%s%s",
                     (int)prefix, line, to, pos + strlen(from));
            strncpy(source_code[line_idx], out, MAX_LEN - 1);
            source_code[line_idx][MAX_LEN - 1] = '\0';
        }
    }
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
                    fix_missing_semicolon(line - 1);
                }
            } else if (strcmp(err_type, "UNDECLARED_VARIABLE") == 0) {
                (void)fscanf(efp, "%d %49s", &line, symbol);
            } else if (strcmp(err_type, "UNUSED_VARIABLE") == 0) {
                (void)fscanf(efp, "%d %49s", &line, symbol);
            }
        }
        fclose(efp);
    } else {
        printf("Warning: Cannot open errors file %s, applying formatting only.\n", errors_file);
    }

    fix_missing_closing_braces_at_eof();

    save_corrected_code(output_file);
    printf("Corrected code saved to %s\n", output_file);

    return 0;
}

#include <stdio.h>
extern FILE* icg_out;
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "errors.h"

int temp_count = 0;
int label_count = 0;

char* new_temp() {
    char *t = (char*)malloc(20);
    if (!t) {
        error_log(ERR_PHASE_ICG, ERR_SEV_FATAL, 0, 0, "OOM_TEMP", "Abort ICG generation.", "Out of memory while allocating temporary variable.");
        return NULL;
    }
    sprintf(t, "t%d", temp_count++);
    return t;
}

char* new_label() {
    char *l = (char*)malloc(20);
    if (!l) {
        error_log(ERR_PHASE_ICG, ERR_SEV_FATAL, 0, 0, "OOM_LABEL", "Abort ICG generation.", "Out of memory while allocating label.");
        return NULL;
    }
    sprintf(l, "L%d", label_count++);
    return l;
}

void generate_ICG(Node* root) {
    if (!icg_out) {
        error_log(ERR_PHASE_ICG, ERR_SEV_FATAL, 0, 0, "ICG_OUTPUT_NULL", "Stop ICG for this translation unit.", "ICG output stream is not initialized.");
        return;
    }
    if (root == NULL || strcmp(root->token, "dummy") == 0 || strcmp(root->token, AST_ERROR_TOKEN) == 0) return;
    if (root->left == NULL && root->right == NULL) {
        strcpy(root->addr, root->token);
        return;
    }
    if (strcmp(root->token, "if") == 0) {
        char *label_end = new_label();
        generate_ICG(root->left);
        if (!root->left) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MISSING_IF_COND", "Skip malformed if-statement TAC emission.", "Missing condition in if node.");
            return;
        }
        fprintf(icg_out, "if %s == 0 goto %s\n", root->left->addr, label_end ? label_end : "LERR");
        generate_ICG(root->right);
        fprintf(icg_out, "%s:\n", label_end ? label_end : "LERR");
        return;
    } else if (strcmp(root->token, "while") == 0) {
        char *label_start = new_label();
        char *label_end = new_label();
        fprintf(icg_out, "%s:\n", label_start);
        generate_ICG(root->left);
        if (!root->left) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MISSING_WHILE_COND", "Skip malformed while TAC emission.", "Missing condition in while node.");
            return;
        }
        fprintf(icg_out, "if %s == 0 goto %s\n", root->left->addr, label_end ? label_end : "LERR");
        generate_ICG(root->right);
        fprintf(icg_out, "goto %s\n", label_start);
        fprintf(icg_out, "%s:\n", label_end ? label_end : "LERR");
        return;
    } else if (strcmp(root->token, "for") == 0) {
        generate_ICG(root->left);
        char *label_start = new_label();
        char *label_end = new_label();
        fprintf(icg_out, "%s:\n", label_start ? label_start : "LERR");
        generate_ICG(root->mid1);
        if (!root->mid1) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MISSING_FOR_COND", "Treat malformed for-loop as no-op.", "Missing condition in for node.");
            return;
        }
        fprintf(icg_out, "if %s == 0 goto %s\n", root->mid1->addr, label_end ? label_end : "LERR");
        generate_ICG(root->right);
        generate_ICG(root->mid2);
        fprintf(icg_out, "goto %s\n", label_start ? label_start : "LERR");
        fprintf(icg_out, "%s:\n", label_end ? label_end : "LERR");
        return;
    } else if (strcmp(root->token, "return") == 0) {
        if (root->left) {
            generate_ICG(root->left);
            fprintf(icg_out, "return %s\n", root->left->addr);
        } else {
            fprintf(icg_out, "return\n");
        }
        return;
    } else if (strcmp(root->token, "param") == 0) {
        generate_ICG(root->left);
        fprintf(icg_out, "param %s\n", root->left->addr);
        return;
    } else if (strcmp(root->token, "param_list") == 0) {
        generate_ICG(root->left);
        generate_ICG(root->right);
        return;
    } else if (strcmp(root->token, "call") == 0) {
        if (root->right) generate_ICG(root->right);
        generate_ICG(root->left);
        char* t = new_temp();
        if (!t || !root->left) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_CALL", "Skip malformed call node.", "Cannot emit TAC for malformed call.");
            return;
        }
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = call %s\n", t, root->left->addr);
        return;
    } else if (strcmp(root->token, "[]") == 0) {
        generate_ICG(root->left);
        generate_ICG(root->right);
        char* t = new_temp();
        if (!t || !root->left || !root->right) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_INDEX", "Skip malformed array index node.", "Cannot emit TAC for malformed array access.");
            return;
        }
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = %s[%s]\n", t, root->left->addr, root->right->addr);
        return;
    } else if (strcmp(root->token, "addr") == 0) {
        generate_ICG(root->left);
        char* t = new_temp();
        if (!t || !root->left) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_ADDR", "Skip malformed address-of node.", "Cannot emit TAC for malformed address operator.");
            return;
        }
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = &%s\n", t, root->left->addr);
        return;
    } else if (strcmp(root->token, "deref") == 0) {
        generate_ICG(root->left);
        char* t = new_temp();
        if (!t || !root->left) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_DEREF", "Skip malformed dereference node.", "Cannot emit TAC for malformed dereference operator.");
            return;
        }
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = *%s\n", t, root->left->addr);
        return;
    }

    generate_ICG(root->left);
    generate_ICG(root->right);
    if (strcmp(root->token, "+") == 0 || strcmp(root->token, "-") == 0 ||
        strcmp(root->token, "*") == 0 || strcmp(root->token, "/") == 0 ||
        strcmp(root->token, ">") == 0 || strcmp(root->token, "<") == 0 ||
        strcmp(root->token, ">=") == 0 || strcmp(root->token, "<=") == 0 ||
        strcmp(root->token, "==") == 0 || strcmp(root->token, "!=") == 0) {
        char *t = new_temp();
        if (!t || !root->left || !root->right) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_BINARY", "Skip malformed expression emission.", "Binary expression node is incomplete.");
            return;
        }
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = %s %s %s\n", root->addr, root->left->addr, root->token, root->right->addr);
    }
    else if (strcmp(root->token, "=") == 0) {
        if (!root->left || !root->right) {
            error_log(ERR_PHASE_ICG, ERR_SEV_ERROR, 0, 0, "MALFORMED_ASSIGN", "Skip malformed assignment emission.", "Assignment node is incomplete.");
            return;
        }
        fprintf(icg_out, "%s = %s\n", root->left->addr, root->right->addr);
        strcpy(root->addr, root->left->addr);
    }
}

#include <stdio.h>
extern FILE* icg_out;
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int temp_count = 0;
int label_count = 0;

char* new_temp() {
    char *t = (char*)malloc(20);
    sprintf(t, "t%d", temp_count++);
    return t;
}

char* new_label() {
    char *l = (char*)malloc(20);
    sprintf(l, "L%d", label_count++);
    return l;
}

void generate_ICG(Node* root) {
    if (root == NULL || strcmp(root->token, "dummy") == 0) return;
    if (root->left == NULL && root->right == NULL) {
        strcpy(root->addr, root->token);
        return;
    }
    if (strcmp(root->token, "if") == 0) {
        char *label_end = new_label();
        generate_ICG(root->left); 
        fprintf(icg_out, "if %s == 0 goto %s\n", root->left->addr, label_end);
        generate_ICG(root->right);
        fprintf(icg_out, "%s:\n", label_end);
        return;
    } else if (strcmp(root->token, "while") == 0) {
        char *label_start = new_label();
        char *label_end = new_label();
        fprintf(icg_out, "%s:\n", label_start);
        generate_ICG(root->left);
        fprintf(icg_out, "if %s == 0 goto %s\n", root->left->addr, label_end);
        generate_ICG(root->right);
        fprintf(icg_out, "goto %s\n", label_start);
        fprintf(icg_out, "%s:\n", label_end);
        return;
    } else if (strcmp(root->token, "for") == 0) {
        generate_ICG(root->left); // init
        char *label_start = new_label();
        char *label_end = new_label();
        fprintf(icg_out, "%s:\n", label_start);
        generate_ICG(root->mid1); // cond
        fprintf(icg_out, "if %s == 0 goto %s\n", root->mid1->addr, label_end);
        generate_ICG(root->right); // body
        generate_ICG(root->mid2); // inc
        fprintf(icg_out, "goto %s\n", label_start);
        fprintf(icg_out, "%s:\n", label_end);
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
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = call %s\n", t, root->left->addr);
        return;
    } else if (strcmp(root->token, "[]") == 0) {
        generate_ICG(root->left);
        generate_ICG(root->right);
        char* t = new_temp();
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = %s[%s]\n", t, root->left->addr, root->right->addr);
        return;
    } else if (strcmp(root->token, "addr") == 0) {
        generate_ICG(root->left);
        char* t = new_temp();
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = &%s\n", t, root->left->addr);
        return;
    } else if (strcmp(root->token, "deref") == 0) {
        generate_ICG(root->left);
        char* t = new_temp();
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
        strcpy(root->addr, t);
        fprintf(icg_out, "%s = %s %s %s\n", root->addr, root->left->addr, root->token, root->right->addr);
    } 
    else if (strcmp(root->token, "=") == 0) {
        fprintf(icg_out, "%s = %s\n", root->left->addr, root->right->addr);
        strcpy(root->addr, root->left->addr);
    }
}
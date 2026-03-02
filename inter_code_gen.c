#include <stdio.h>
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
        printf("if %s == 0 goto %s\n", root->left->addr, label_end);
        generate_ICG(root->right);
        printf("%s:\n", label_end);
        return;
    } else if (strcmp(root->token, "while") == 0) {
        char *label_start = new_label();
        char *label_end = new_label();
        printf("%s:\n", label_start);
        generate_ICG(root->left);
        printf("if %s == 0 goto %s\n", root->left->addr, label_end);
        generate_ICG(root->right);
        printf("goto %s\n", label_start);
        printf("%s:\n", label_end);
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
        printf("%s = %s %s %s\n", root->addr, root->left->addr, root->token, root->right->addr);
    } 
    else if (strcmp(root->token, "=") == 0) {
        printf("%s = %s\n", root->left->addr, root->right->addr);
        strcpy(root->addr, root->left->addr);
    }
}
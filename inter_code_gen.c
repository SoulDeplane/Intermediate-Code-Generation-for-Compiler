#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
    struct Node *left;
    struct Node *right;
    char token[100];
    char addr[50]; 
} Node;

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
    if (root == NULL) return;

    
    if (root->left == NULL && root->right == NULL) {
        strcpy(root->addr, root->token);
        return;
    }

    
    generate_ICG(root->left);
    generate_ICG(root->right);

    
    if (strcmp(root->token, "+") == 0 || strcmp(root->token, "-") == 0 || 
        strcmp(root->token, "*") == 0 || strcmp(root->token, "/") == 0) {
        
        char *t = new_temp();
        strcpy(root->addr, t);
        printf("%s = %s %s %s\n", root->addr, root->left->addr, root->token, root->right->addr);
    } 
    
    else if (strcmp(root->token, "=") == 0) {
        printf("%s = %s\n", root->left->addr, root->right->addr);
        strcpy(root->addr, root->left->addr);
    }
    else if (strcmp(root->token, "if") == 0 || strcmp(root->token, "while") == 0) {
        char *label = new_label();
        printf("IF NOT %s GOTO %s\n", root->left->addr, label);
        printf("%s:\n", label);
    }
}
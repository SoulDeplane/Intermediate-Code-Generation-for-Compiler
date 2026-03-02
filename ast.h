#ifndef AST_H
#define AST_H

typedef struct Node {
    struct Node *left;
    struct Node *right;
    char token[100];
    char addr[50];
} Node;

void create_node(char *token, int leaf);
void push_tree(Node *newnode);
Node *pop_tree();
void generate_ICG(Node* root);
void report_error(int line, const char* msg);
#endif
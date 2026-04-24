#ifndef AST_H
#define AST_H

#define AST_ERROR_TOKEN "__error__"

typedef struct Node {
    struct Node *left;
    struct Node *mid1;
    struct Node *mid2;
    struct Node *right;
    char token[100];
    char addr[50];
    int eval_type;
} Node;

void create_node(char *token, int leaf);
void push_tree(Node *newnode);
Node *pop_tree();
void generate_ICG(Node* root);
void report_error(int line, const char* msg);
void clear_tree(Node* root);
#endif

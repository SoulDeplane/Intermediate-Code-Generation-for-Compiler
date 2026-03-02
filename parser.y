%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <limits.h>
    #include "ast.h"

    void yyerror(const char*);
    int yylex();
    extern FILE * yyin, *yyout;
    extern int lno;
    
    int scope = 0;
    int datatype = -1;
    char tempStr[100];
    int error_occurred = 0;

    struct node {
        char name[20];
        int dtype;
        int scope;
        int valid;
        union value {
            float f;
            int i;
            char c;
        } val;
        struct node *link;
    } *first = NULL;

    typedef struct tree_stack {
        Node *node;
        struct tree_stack *next;
    } tree_stack;
    tree_stack *tree_top = NULL;

    struct node* lookup(char *name);
    struct node* insert(char *name, int type);
    void printsymtable();
    void cleansymbol();
    
    void printAST(Node* root, int level);
    void clear_tree(Node* root);
%}

%union {
    int ival;
    float fval;
    char *sval;
    struct node *ptr;
}

%token <sval> IDENTIFIER STRING_LITERAL CHAR_LITERAL
%token <ival> NUMBER
%token <fval> FLOAT_VALUE
%token CHAR FLOAT INT VOID FOR WHILE IF RETURN AUTO BREAK CASE CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM EXTERN GOTO LONG REGISTER SHORT SIGNED SIZEOF STATIC STRUCT SWITCH TYPEDEF UNION UNSIGNED VOLATILE
%token INLINE RESTRICT BOOL COMPLEX IMAGINARY ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL
%token BOOL_KEYWORD TRUE_KEYWORD FALSE_KEYWORD NULLPTR_KEYWORD
%token EQ NE LE GE INC DEC ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN AND OR AND_ASSIGN OR_ASSIGN XOR_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN LEFT_OP RIGHT_OP ELLIPSIS ARROW BIT_AND BIT_OR BIT_XOR
%token ASSIGN PLUS MINUS MULT DIV MOD NOT BIT_NOT QUESTION LT GT
%token COLON SEMICOLON COMMA
%token LBRACE RBRACE LPAREN RPAREN LBRACK RBRACK DOT

%left PLUS MINUS
%left DIV MULT MOD
%right ASSIGN

%type <fval> assignment_expression primary_expression equality_expression 
%type <fval> relational_expression additive_expression multiplicative_expression 
%type <fval> unary_expression conditional_expression expression expression_statement 
%type <fval> postfix_expression

%start S

%%
S : program { 
    if(!error_occurred) {
        printf("\n--- Intermediate Code (TAC) ---\n");
        if(tree_top != NULL) {
            generate_ICG(tree_top->node);
        }

        printf("\n--- Abstract Syntax Tree ---\n");
        while(tree_top != NULL) {
            Node *root = pop_tree();
            printAST(root, 0);
            clear_tree(root);
        }
    } else {
        printf("\n--- Compilation Halted Due to Errors ---\n");
    }
    printsymtable(); 
    cleansymbol(); 
}

program : translation_unit ;
translation_unit : ext_dec | translation_unit ext_dec ;
ext_dec : declaration | function_definition | statement ;

function_definition 
    : type_specifier IDENTIFIER LPAREN parameter_list RPAREN compound_statement { create_node("func_def", 0); }
    ;

parameter_list 
    : 
    | parameter_declaration
    | parameter_list COMMA parameter_declaration
    ;

parameter_declaration : type_specifier IDENTIFIER { 
    if (insert($2, datatype) == NULL) {
        /* Error already thrown in insert */
    }
} ;

compound_statement 
    : LBRACE { scope++; } block_item_list RBRACE {
        struct node *ftp = first;
        while(ftp != NULL) {
            if(ftp->scope == scope && ftp->valid == 1) ftp->valid = 0;
            ftp = ftp->link;
        }
        scope--;
    }
    | LBRACE RBRACE
    ;

block_item_list : block_item | block_item_list block_item { create_node("stmt_seq", 0); } ;
block_item : declaration | statement | RETURN expression_statement { create_node("return", 1); } ;

declaration : type_specifier init_declarator_list SEMICOLON ;

statement 
    : compound_statement 
    | expression_statement 
    | iteration_statement 
    | condition_statement
    | error SEMICOLON { yyerror("Invalid statement; please check your syntax before the semicolon."); yyerrok; }
    ;

condition_statement 
    : IF LPAREN relational_expression RPAREN statement { 
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *ifnode = (Node*)malloc(sizeof(Node));
        strcpy(ifnode->token, "if");
        ifnode->left = cond;
        ifnode->right = stmt;
        push_tree(ifnode);
    } 
    ;

iteration_statement 
    : FOR LPAREN expression_statement expression_statement expression RPAREN statement { 
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *fornode = (Node*)malloc(sizeof(Node));
        strcpy(fornode->token, "for");
        fornode->left = cond;
        fornode->right = stmt;
        push_tree(fornode);
    }
    | WHILE LPAREN relational_expression RPAREN statement { 
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *whilenode = (Node*)malloc(sizeof(Node));
        strcpy(whilenode->token, "while");
        whilenode->left = cond;
        whilenode->right = stmt;
        push_tree(whilenode);
    }
    ;

type_specifier 
    : INT   { datatype = 0; }
    | FLOAT { datatype = 1; }
    | CHAR  { datatype = 2; }
    | VOID  { datatype = 3; }
    ;

init_declarator_list : init_declarator | init_declarator_list COMMA init_declarator ;

init_declarator
    : IDENTIFIER { 
        struct node* res = insert($1, datatype);
        if (res) {
            create_node(res->name, 1); 
        } else {
            create_node("dummy", 1);
        }
        $<ptr>$ = res; 
    } ASSIGN assignment_expression {
        struct node* id_ptr = $<ptr>2;
        if (id_ptr) {
            if (id_ptr->dtype == 0) id_ptr->val.i = (int)$4;
            else if (id_ptr->dtype == 1) id_ptr->val.f = $4;
            create_node("=", 0);
        } else {
            Node* expr = pop_tree();
            Node* dummy = pop_tree();
            clear_tree(expr);
            clear_tree(dummy);
            create_node("dummy", 1);
        }
    }
    | IDENTIFIER {
        struct node* res = insert($1, datatype);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
    }
    ;

primary_expression 
    : IDENTIFIER {
        struct node* res = lookup($1);
        if(!res) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
            $$ = 0;
            create_node("dummy", 1);
        } else {
            if(res->dtype == 0) $$ = (float)res->val.i;
            else if(res->dtype == 1) $$ = res->val.f;
            create_node(res->name, 1);
        }
    }
    | NUMBER { $$ = (float)$1; sprintf(tempStr, "%d", $1); create_node(tempStr, 1); }
    | FLOAT_VALUE { $$ = $1; sprintf(tempStr, "%f", $1); create_node(tempStr, 1); }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

postfix_expression : primary_expression ;
unary_expression : postfix_expression ;

multiplicative_expression 
    : unary_expression
    | multiplicative_expression MULT unary_expression { $$ = $1 * $3; create_node("*", 0); }
    | multiplicative_expression DIV unary_expression { 
        if($3 == 0) {
            report_error(lno, "Division by zero");
        }
        $$ = $1 / $3; create_node("/", 0); 
    }
    | multiplicative_expression MOD unary_expression { $$ = (int)$1 % (int)$3; create_node("%", 0); }
    ;

additive_expression 
    : multiplicative_expression
    | additive_expression PLUS multiplicative_expression { $$ = $1 + $3; create_node("+", 0); }
    | additive_expression MINUS multiplicative_expression { $$ = $1 - $3; create_node("-", 0); }
    ;

relational_expression 
    : additive_expression
    | relational_expression LE additive_expression { $$ = $1 <= $3; create_node("<=", 0); }
    | relational_expression GE additive_expression { $$ = $1 >= $3; create_node(">=", 0); }
    | relational_expression LT additive_expression { $$ = $1 < $3; create_node("<", 0); }
    | relational_expression GT additive_expression { $$ = $1 > $3; create_node(">", 0); }
    | relational_expression EQ additive_expression { $$ = $1 == $3; create_node("==", 0); }
    | relational_expression NE additive_expression { $$ = $1 != $3; create_node("!=", 0); }
    ;

equality_expression : relational_expression ;
conditional_expression : equality_expression ;
assignment_expression 
    : conditional_expression 
    | IDENTIFIER { create_node($1, 1); } ASSIGN assignment_expression {
        struct node* res = lookup($1);
        if(!res) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
            $$ = 0;
        } else {
            if(res->dtype == 0) res->val.i = (int)$4;
            else if(res->dtype == 1) res->val.f = $4;
            $$ = $4;
        }
        create_node("=", 0);
    }
    ;
expression : assignment_expression ;
expression_statement : SEMICOLON { $$ = 0; } | expression SEMICOLON { $$ = $1; };

%%
int main() {
    printf("Starting modern C parser...\n");
    return yyparse();
}

void report_error(int line, const char* msg) {
    if(!error_occurred) error_occurred = 1;
    fprintf(stderr, "[Friendly Compiler Notice] Error at line %d: %s\n", line, msg);
}

void yyerror(const char* s) {
    report_error(lno, s);
}

void printdebug(const char* msg, char c) {
    if (c != ' ') printf("Debug: %s '%c' at line %d\n", msg, c, lno);
    else printf("Debug: %s at line %d\n", msg, lno);
}

struct node* lookup(char *name) {
    struct node *ptr = first;
    while(ptr != NULL) {
        if(strcmp(ptr->name, name) == 0 && ptr->valid == 1) return ptr;
        ptr = ptr->link;
    }
    return NULL;
}

struct node* insert(char *name, int type) {
    struct node *ptr = first;
    while(ptr != NULL) {
        if(strcmp(ptr->name, name) == 0 && ptr->scope == scope && ptr->valid == 1) {
            char errmsg[50];
            sprintf(errmsg, "Redeclaration of %s", name);
            report_error(lno, errmsg);
            return NULL;
        }
        ptr = ptr->link;
    }
    struct node *newnode = (struct node*)malloc(sizeof(struct node));
    strcpy(newnode->name, name);
    newnode->dtype = type;
    newnode->scope = scope;
    newnode->valid = 1;
    newnode->link = first;
    first = newnode;
    return newnode;
}

void printsymtable() {
    printf("\n--- Symbol Table ---\n");
    struct node *ptr = first;
    while(ptr != NULL) {
        printf("Name: %-10s | Scope: %d | Type: %d | Status: %s\n", 
               ptr->name, ptr->scope, ptr->dtype, ptr->valid ? "Active" : "Dead");
        ptr = ptr->link;
    }
}

void cleansymbol() {
    struct node *ptr = first;
    while(ptr != NULL) {
        struct node *del = ptr;
        ptr = ptr->link;
        free(del);
    }
    first = NULL;
}

void push_tree(Node *newnode) {
    tree_stack *temp = (tree_stack*)malloc(sizeof(tree_stack));
    temp->node = newnode;
    temp->next = tree_top;
    tree_top = temp;
}

Node *pop_tree() {
    if(tree_top == NULL) return NULL;
    tree_stack *temp = tree_top;
    Node *ret = temp->node;
    tree_top = tree_top->next;
    free(temp);
    return ret;
}

void create_node(char *token, int leaf) {
    Node *newnode = (Node*)malloc(sizeof(Node));
    strcpy(newnode->token, token);
    if (leaf) {
        newnode->left = newnode->right = NULL;
    } else {
        newnode->right = pop_tree();
        newnode->left = pop_tree();
    }
    push_tree(newnode);
}

void printAST(Node* root, int level) {
    if (root == NULL || strcmp(root->token, "dummy") == 0) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("-> %s\n", root->token);
    printAST(root->left, level + 1);
    printAST(root->right, level + 1);
}

void clear_tree(Node* root) {
    if (root == NULL) return;
    clear_tree(root->left);
    clear_tree(root->right);
    free(root);
}
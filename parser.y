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
    FILE *icg_out = NULL;

    int scope = 0;
    int datatype = -1;
    char tempStr[100];
    int error_occurred = 0;

    struct node {
        char name[20];
        int dtype;
        int scope;
        int valid;
        int is_used;
        union value {
            float f;
            int i;
            char c;
        } val;
        int is_allocated;
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

%type <ptr> assignment_expression primary_expression equality_expression
%type <ptr> relational_expression additive_expression multiplicative_expression
%type <ptr> unary_expression conditional_expression expression expression_statement
%type <ptr> postfix_expression argument_expression_list

%start S

%%
S : program {
    if(!error_occurred) {
        printf("\n--- Intermediate Code (TAC) ---\n");
        if(tree_top != NULL) {
            icg_out = fopen("icg.txt", "w");
            if (icg_out) {
                generate_ICG(tree_top->node);
                fclose(icg_out);
                printf("Intermediate Code Successfully exported to icg.txt\n");
            } else {
                printf("Failed to open icg.txt for writing.\n");
            }
        }

        printf("\n--- Abstract Syntax Tree ---\n");
        tree_stack *temp = tree_top;
        while(temp != NULL) {
            printAST(temp->node, 0);
            temp = temp->next;
        }
    } else {
        printf("\n--- Compilation Halted Due to Errors ---\n");
    }

    while(tree_top != NULL) {
        Node *root = pop_tree();
        clear_tree(root);
    }
    struct node *ftp = first;
    while(ftp != NULL) {
        if(ftp->valid == 1) {
            if (ftp->is_used == 0) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning: Variable '%s' is declared but never used. Consider removing it to clean up your code.\n", ftp->name);
            }
            if (ftp->is_allocated == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning: Memory leak detected! Variable '%s' was allocated but never freed before execution end.\n", ftp->name);
            }
        }
        ftp = ftp->link;
    }
    printsymtable();
    cleansymbol();
}

program : translation_unit ;
translation_unit : ext_dec | translation_unit ext_dec { create_node("stmt_seq", 0); } ;
ext_dec : declaration | function_definition | statement ;

function_definition
    : type_specifier IDENTIFIER { insert($2, datatype); } LPAREN parameter_list RPAREN compound_statement { create_node("func_def", 0); }
    ;

parameter_list
    :
    | parameter_declaration
    | parameter_list COMMA parameter_declaration
    ;

parameter_declaration : type_specifier IDENTIFIER {
    if (insert($2, datatype) == NULL) {

    }
} ;

compound_statement
    : LBRACE { scope++; } block_item_list RBRACE {
        struct node *ftp = first;
        while(ftp != NULL) {
            if(ftp->scope == scope && ftp->valid == 1) {
                if (ftp->is_used == 0) {
                    fprintf(stderr, "[Friendly Compiler Notice] Warning: Variable '%s' is declared but never used. Consider removing it to clean up your code.\n", ftp->name);
                }
                if (ftp->is_allocated == 1) {
                    fprintf(stderr, "[Friendly Compiler Notice] Warning: Memory leak detected! Variable '%s' was allocated but never freed before leaving scope.\n", ftp->name);
                }
                ftp->valid = 0;
            }
            ftp = ftp->link;
        }
        scope--;
    }
    | LBRACE RBRACE
    ;

block_item_list : block_item | block_item_list block_item { create_node("stmt_seq", 0); } ;
block_item : declaration | statement
    | RETURN expression SEMICOLON {
        Node* expr = pop_tree();
        Node* retNode = (Node*)malloc(sizeof(Node));
        strcpy(retNode->token, "return");
        retNode->left = expr;
        retNode->right = NULL;
        push_tree(retNode);
    }
    | RETURN SEMICOLON {
        Node* retNode = (Node*)malloc(sizeof(Node));
        strcpy(retNode->token, "return");
        retNode->left = NULL;
        retNode->right = NULL;
        push_tree(retNode);
    }
    ;

declaration
    : type_specifier init_declarator_list SEMICOLON
    | type_specifier init_declarator_list error { yyerror("Missing semicolon (;) after variable declaration."); yyerrok; }
    | error init_declarator_list SEMICOLON { yyerror("Missing valid type specifier (int, float, etc) for declaration."); yyerrok; }
    ;

statement
    : compound_statement
    | expression_statement
    | iteration_statement
    | condition_statement
    | error SEMICOLON { yyerror("Invalid statement; please check your syntax before the semicolon."); yyerrok; }
    | expression error { yyerror("Missing semicolon immediately following expression block."); yyerrok; }
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
    | IF error RPAREN statement { yyerror("Missing or malformed condition inside 'if'. Did you forget an opening '('?"); yyerrok; }
    | IF LPAREN relational_expression error { yyerror("Missing closing ')' for 'if' condition."); yyerrok; }
    ;

iteration_statement
    : FOR LPAREN expression_statement expression_statement expression RPAREN statement {
        Node *stmt = pop_tree();
        Node *inc = pop_tree();
        Node *cond = pop_tree();
        Node *init = pop_tree();
        Node *fornode = (Node*)malloc(sizeof(Node));
        strcpy(fornode->token, "for");
        fornode->left = init;
        fornode->mid1 = cond;
        fornode->mid2 = inc;
        fornode->right = stmt;
        push_tree(fornode);
    }
    | FOR error statement { yyerror("Malformed 'for' loop declaration."); yyerrok; }
    | WHILE LPAREN relational_expression RPAREN statement {
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *whilenode = (Node*)malloc(sizeof(Node));
        strcpy(whilenode->token, "while");
        whilenode->left = cond;
        whilenode->right = stmt;
        push_tree(whilenode);
    }
    | WHILE error RPAREN statement { yyerror("Missing or malformed condition inside 'while'. Did you forget an opening '('?"); yyerrok; }
    | WHILE LPAREN relational_expression error { yyerror("Missing closing ')' for 'while' condition."); yyerrok; }
    ;

type_specifier
    : INT   { datatype = 0; }
    | FLOAT { datatype = 1; }
    | CHAR  { datatype = 2; }
    | VOID  { datatype = 3; }
    ;

init_declarator_list : init_declarator | init_declarator_list COMMA init_declarator { create_node("stmt_seq", 0); } ;

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
            if (id_ptr->dtype == 0 && $<ptr>4->dtype == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: Possible loss of data. You are assigning a decimal value to the integer variable '%s'.\n", lno, id_ptr->name);
            } else if ((id_ptr->dtype == 0 || id_ptr->dtype == 1) && ($<ptr>4->dtype == 2)) {
                fprintf(stderr, "[Friendly Compiler Notice] Type Mismatch Error at line %d: Cannot initialize a numeric variable '%s' with a character.\n", lno, id_ptr->name);
                error_occurred = 1;
            } else if ((id_ptr->dtype == 0 || id_ptr->dtype == 1) && $<ptr>4->dtype == 4 && id_ptr->dtype != 4) {

            }
            if (strcmp($<ptr>4->name, "__malloc__") == 0) {
                id_ptr->is_allocated = 1;
            }

            if (id_ptr->dtype == 0) id_ptr->val.i = ($<ptr>4->dtype == 0) ? $<ptr>4->val.i : (int)$<ptr>4->val.f;
            else if (id_ptr->dtype == 1) id_ptr->val.f = ($<ptr>4->dtype == 1) ? $<ptr>4->val.f : (float)$<ptr>4->val.i;
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
    | IDENTIFIER LBRACK NUMBER RBRACK {
        struct node* res = insert($1, datatype);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
    }
    | MULT IDENTIFIER {
        struct node* res = insert($2, 4);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
        $<ptr>$ = res;
    } ASSIGN assignment_expression {
        struct node* id_ptr = $<ptr>2;
        if (id_ptr) {
            if (strcmp($<ptr>4->name, "__malloc__") == 0) {
                id_ptr->is_allocated = 1;
            }
            create_node("=", 0);
        } else {
            Node* expr = pop_tree();
            Node* dummy = pop_tree();
            clear_tree(expr);
            clear_tree(dummy);
            create_node("dummy", 1);
        }
    }
    | MULT IDENTIFIER {
        struct node* res = insert($2, 4);
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

            struct node* dummy = (struct node*)malloc(sizeof(struct node));
            dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
            $$ = dummy;
            create_node("dummy", 1);
        } else {
            res->is_used = 1;

            struct node* ret = (struct node*)malloc(sizeof(struct node));
            memcpy(ret, res, sizeof(struct node));
            $$ = ret;
            create_node(res->name, 1);
        }
    }
    | NUMBER {
        struct node* num_node = (struct node*)malloc(sizeof(struct node));
        num_node->dtype = 0;
        num_node->val.i = $1;
        $$ = num_node;
        sprintf(tempStr, "%d", $1);
        create_node(tempStr, 1);
    }
    | FLOAT_VALUE {
        struct node* f_node = (struct node*)malloc(sizeof(struct node));
        f_node->dtype = 1;
        f_node->val.f = $1;
        $$ = f_node;
        sprintf(tempStr, "%f", $1);
        create_node(tempStr, 1);
    }
    | STRING_LITERAL {
        struct node* s_node = (struct node*)malloc(sizeof(struct node));
        s_node->dtype = 4;
        $$ = s_node;
        create_node("string_lit", 1);
    }
    | CHAR_LITERAL {
        struct node* c_node = (struct node*)malloc(sizeof(struct node));
        c_node->dtype = 2;
        $$ = c_node;
        create_node("char_lit", 1);
    }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

argument_expression_list
    : assignment_expression {
        Node* expr = pop_tree();
        Node* paramNode = (Node*)malloc(sizeof(Node));
        strcpy(paramNode->token, "param");
        paramNode->left = expr;
        paramNode->right = NULL;
        push_tree(paramNode);
        $$ = $1;
    }
    | argument_expression_list COMMA assignment_expression {
        Node* expr = pop_tree();
        Node* list = pop_tree();
        Node* paramNode = (Node*)malloc(sizeof(Node));
        strcpy(paramNode->token, "param");
        paramNode->left = expr;
        paramNode->right = NULL;

        Node* seq = (Node*)malloc(sizeof(Node));
        strcpy(seq->token, "param_list");
        seq->left = list;
        seq->right = paramNode;
        push_tree(seq);
        $$ = $1;
    }
    ;

postfix_expression
    : primary_expression
    | postfix_expression LBRACK expression RBRACK {
        struct node* arr_node = (struct node*)malloc(sizeof(struct node));
        arr_node->dtype = 0;
        $$ = arr_node;
        create_node("[]", 0);
    }
    | IDENTIFIER LPAREN RPAREN {
        if (strcmp($1, "gets") == 0) {
            fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: '%s' is unsafe; it can lead to buffer overflows. Consider using 'fgets' instead.\n", lno, $1);
        }

        struct node* res = lookup($1);
        if(!res &&
           strcmp($1, "gets") != 0 && strcmp($1, "malloc") != 0 && strcmp($1, "free") != 0 &&
           strcmp($1, "printf") != 0 && strcmp($1, "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
        }

        Node* callNode = (Node*)malloc(sizeof(Node));
        strcpy(callNode->token, "call");

        Node* funcNode = (Node*)malloc(sizeof(Node));
        strcpy(funcNode->token, $1);
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = NULL;
        push_tree(callNode);

        struct node* call_res = (struct node*)malloc(sizeof(struct node));
        if (strcmp($1, "malloc") == 0) strcpy(call_res->name, "__malloc__");
        call_res->dtype = 0;
        $$ = call_res;
    }
    | IDENTIFIER LPAREN argument_expression_list RPAREN {
        if (strcmp($1, "gets") == 0) {
            fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: '%s' is unsafe; it can lead to buffer overflows. Consider using 'fgets' instead.\n", lno, $1);
        }

        if (strcmp($1, "free") == 0) {
            if ($3 && $3->is_allocated == 1) {

                struct node* free_target = lookup($3->name);
                if (free_target) free_target->is_allocated = 0;
            }
        }

        struct node* res = lookup($1);
        if(!res &&
           strcmp($1, "gets") != 0 && strcmp($1, "malloc") != 0 && strcmp($1, "free") != 0 &&
           strcmp($1, "printf") != 0 && strcmp($1, "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
        }
        Node* args = pop_tree();
        Node* callNode = (Node*)malloc(sizeof(Node));
        strcpy(callNode->token, "call");

        Node* funcNode = (Node*)malloc(sizeof(Node));
        strcpy(funcNode->token, $1);
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = args;
        push_tree(callNode);

        struct node* call_res = (struct node*)malloc(sizeof(struct node));
        if (strcmp($1, "malloc") == 0) strcpy(call_res->name, "__malloc__");
        call_res->dtype = 0;
        $$ = call_res;
    }
    ;

unary_expression
    : postfix_expression
    | BIT_AND unary_expression {
         struct node* res = (struct node*)malloc(sizeof(struct node));
         res->dtype = 4;
         $$ = res;

         Node* operand = pop_tree();
         Node* opNode = (Node*)malloc(sizeof(Node));
         strcpy(opNode->token, "addr");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
    | MULT unary_expression {
         struct node* res = (struct node*)malloc(sizeof(struct node));
         res->dtype = 4;
         $$ = res;

         Node* operand = pop_tree();
         Node* opNode = (Node*)malloc(sizeof(Node));
         strcpy(opNode->token, "deref");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression MULT unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform multiplication on strings or characters."); }
        $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("*", 0);
    }
    | multiplicative_expression DIV unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform division on strings or characters."); }
        float rhs = ($3->dtype == 0) ? (float)$3->val.i : $3->val.f;
        if(rhs == 0.0) {
            report_error(lno, "Division by zero");
        }
        $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("/", 0);
    }
    | multiplicative_expression MOD unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot modulo strings or characters."); }
        $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("%", 0);
    }
    ;

additive_expression
    : multiplicative_expression
    | additive_expression PLUS multiplicative_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly add strings or characters."); }
        $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("+", 0);
    }
    | additive_expression MINUS multiplicative_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly subtract strings or characters."); }
        $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("-", 0);
    }
    ;

relational_expression
    : additive_expression
    | relational_expression LE additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("<=", 0); }
    | relational_expression GE additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node(">=", 0); }
    | relational_expression LT additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("<", 0); }
    | relational_expression GT additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node(">", 0); }
    | relational_expression EQ additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("==", 0); }
    | relational_expression NE additive_expression { $$ = (struct node*)malloc(sizeof(struct node)); $$->dtype = 0; create_node("!=", 0); }
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

            struct node* dummy = (struct node*)malloc(sizeof(struct node));
            dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
            $$ = dummy;
        } else {
            if (res->dtype == 0 && $4->dtype == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: Possible loss of data. You are assigning a decimal value to the integer variable '%s'.\n", lno, res->name);
            } else if ((res->dtype == 0 || res->dtype == 1) && ($4->dtype == 2)) {
                fprintf(stderr, "[Friendly Compiler Notice] Type Mismatch Error at line %d: Cannot assign a character to a numeric variable '%s'.\n", lno, res->name);
                error_occurred = 1;
            } else if ((res->dtype == 0 || res->dtype == 1) && $4->dtype == 4 && res->dtype != 4) {

            }
            if (strcmp($4->name, "__malloc__") == 0) {
                res->is_allocated = 1;
            }
            if ($4->is_allocated == 1 && strcmp($4->name, res->name) != 0) {

                res->is_allocated = 1;
            }

            if(res->dtype == 0) res->val.i = ($4->dtype == 0) ? $4->val.i : (int)$4->val.f;
            else if(res->dtype == 1) res->val.f = ($4->dtype == 1) ? $4->val.f : (float)$4->val.i;
            $$ = $4;
        }
        create_node("=", 0);
    }
    ;
expression : assignment_expression ;
expression_statement : SEMICOLON {
    struct node* dummy = (struct node*)malloc(sizeof(struct node));
    dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
    $$ = dummy;
} | expression SEMICOLON { $$ = $1; };

%%
int main() {
    printf("Starting modern C parser...\n");
    printf("\n--- Lexical Tokens ---\n");
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
    if (type == 3) {
        char errmsg[100];
        sprintf(errmsg, "Variable '%s' cannot be declared as type 'void'", name);
        report_error(lno, errmsg);
        return NULL;
    }
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
    newnode->is_used = 0;
    newnode->is_allocated = 0;
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
    newnode->mid1 = NULL;
    newnode->mid2 = NULL;
    if (leaf) {
        newnode->left = newnode->right = NULL;
    } else {
        newnode->right = pop_tree();
        newnode->left = pop_tree();
    }

    newnode->eval_type = tree_top && tree_top->node ? tree_top->node->eval_type : 0;
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

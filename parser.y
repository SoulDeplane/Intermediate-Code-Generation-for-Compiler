%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <limits.h>
    #include <setjmp.h>
    #include "ast.h"
    #include "errors.h"

    void yyerror(const char*);
    int yylex();
    extern FILE * yyin, *yyout;
    extern int lno;
    extern int colno;
    FILE *icg_out = NULL;

    int scope = 0;
    int datatype = -1;
    char tempStr[100];
    int error_occurred = 0;
    int current_function_return_type = -1;
    int current_function_has_return = 0;
    char current_function_name[20] = {0};
    int current_insert_kind = 0;
    #define SYM_KIND_VAR 0
    #define SYM_KIND_FUNC 1
    #define SYM_KIND_PARAM 2

    #define MAX_PARAMS 32

    typedef struct function_sig {
        char name[20];
        int return_type;
        int param_count;
        int param_types[MAX_PARAMS];
        struct function_sig *next;
    } function_sig;
    function_sig *functions = NULL;
    int pending_param_types[MAX_PARAMS];
    int pending_param_count = 0;

    struct node {
        char name[20];
        int dtype;
        int scope;
        int valid;
        int is_used;
        int sym_kind;
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

    void clear_tree(Node* root);
    static void* checked_calloc(size_t count, size_t size);
    static struct node* new_semantic_node(int dtype, const char *name);
    static Node* new_ast_node(const char *token);
    static function_sig* lookup_function(const char *name);
    static void upsert_function_signature(const char *name, int return_type, int param_count, int *param_types);
    static const char* dtype_name(int dtype);

    typedef struct LexToken {
        char type[24];
        char text[128];
        int line;
    } LexToken;
    extern LexToken token_buf[];
    extern int token_buf_count;

    typedef struct CSTNode {
        char label[80];
        int is_leaf;
        struct CSTNode **children;
        int n_children;
        int cap;
    } CSTNode;
    static CSTNode *cst_parse_program(void);
    static void cst_print(CSTNode *n, const char *prefix, int is_last, int is_root);
    static void cst_print_json(CSTNode *n, FILE *out);
    static void cst_free(CSTNode *n);
%}

%code requires {
    #ifndef ARG_LIST_INFO_DECLARED
    #define ARG_LIST_INFO_DECLARED
    typedef struct arg_list_info {
        int count;
        int types[32];
        int is_allocated[32];
        char names[32][20];
    } arg_list_info;
    #endif
}

%union {
    int ival;
    float fval;
    char *sval;
    struct node *ptr;
    arg_list_info *alist;
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
%type <ptr> postfix_expression
%type <alist> argument_expression_list

%start S

%%
S : program {
    if(!error_has_errors() && tree_top != NULL) {
        icg_out = fopen("icg.txt", "w");
        if (icg_out) {
            generate_ICG(tree_top->node);
            fclose(icg_out);
        }
    }

    printf("\n--- Parser Tree ---\n");
    {
        CSTNode *cst_root = cst_parse_program();
        cst_print(cst_root, "", 1, 1);
        printf("\n--- Parser Tree JSON ---\n");
        cst_print_json(cst_root, stdout);
        printf("\n");
        cst_free(cst_root);
    }

    printf("\n--- Lexical Tokens JSON ---\n[");
    for (int i = 0; i < token_buf_count; i++) {
        if (i) fputc(',', stdout);
        printf("{\"line\":%d,\"type\":\"%s\",\"text\":\"",
               token_buf[i].line, token_buf[i].type);
        for (const char *p = token_buf[i].text; *p; p++) {
            unsigned char c = (unsigned char)*p;
            if (c == '"' || c == '\\') { fputc('\\', stdout); fputc(c, stdout); }
            else if (c == '\n') fputs("\\n", stdout);
            else if (c == '\r') fputs("\\r", stdout);
            else if (c == '\t') fputs("\\t", stdout);
            else if (c < 0x20) printf("\\u%04x", c);
            else fputc(c, stdout);
        }
        fputs("\"}", stdout);
    }
    printf("]\n");

    while(tree_top != NULL) {
        Node *root = pop_tree();
        clear_tree(root);
    }
    struct node *ftp = first;
    while(ftp != NULL) {
        if(ftp->valid == 1) {
            if (ftp->is_used == 0 && ftp->sym_kind == SYM_KIND_VAR && strcmp(ftp->name, "main") != 0) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "UNUSED_VARIABLE", "Continue with symbol table checks", "Variable '%s' is declared but never used.", ftp->name);
            }
            if (ftp->is_allocated == 1) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "LEAK_POSSIBLE", "Continue with leak advisory", "Memory leak detected. Variable '%s' was allocated but never freed before execution end.", ftp->name);
            }
        }
        ftp = ftp->link;
    }
    printsymtable();
    cleansymbol();
    while (functions != NULL) {
        function_sig *tmp = functions;
        functions = functions->next;
        free(tmp);
    }
    error_print_summary(stderr);
}

program : translation_unit ;
translation_unit : ext_dec | translation_unit ext_dec { create_node("stmt_seq", 0); } ;
ext_dec : declaration | function_definition | statement ;

function_definition
    : type_specifier IDENTIFIER {
        current_insert_kind = SYM_KIND_FUNC;
        insert($2, datatype);
        current_insert_kind = SYM_KIND_VAR;
        current_function_return_type = datatype;
        current_function_has_return = 0;
        snprintf(current_function_name, sizeof(current_function_name), "%s", $2);
        pending_param_count = 0;
    } LPAREN parameter_list RPAREN compound_statement {
        upsert_function_signature(current_function_name, current_function_return_type, pending_param_count, pending_param_types);
        if (current_function_return_type != 3 && !current_function_has_return) {
            error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "MISSING_RETURN", "Insert `return <value>;` in all non-void paths.", "Non-void function '%s' may reach end without returning a value.", current_function_name);
            error_occurred = 1;
        }
        create_node("func_def", 0);
    }
    ;

parameter_list
    :
    | parameter_declaration
    | parameter_list COMMA parameter_declaration
    ;

parameter_declaration : type_specifier IDENTIFIER {
    current_insert_kind = SYM_KIND_PARAM;
    if (insert($2, datatype) == NULL) {

    }
    current_insert_kind = SYM_KIND_VAR;
    if (pending_param_count < MAX_PARAMS) {
        pending_param_types[pending_param_count++] = datatype;
    } else {
        error_log(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "TOO_MANY_PARAMS", "Only first 32 parameters are tracked for signature checks.", "Too many function parameters for signature checker.");
        error_occurred = 1;
    }
} ;

compound_statement
    : LBRACE { scope++; } block_item_list RBRACE {
        struct node *ftp = first;
        while(ftp != NULL) {
            if(ftp->scope == scope && ftp->valid == 1) {
                if (ftp->is_used == 0 && ftp->sym_kind == SYM_KIND_VAR) {
                    error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "UNUSED_VARIABLE", "Scope cleanup continues", "Variable '%s' is declared but never used.", ftp->name);
                }
                if (ftp->is_allocated == 1) {
                    error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "LEAK_POSSIBLE", "Scope cleanup continues", "Memory leak detected. Variable '%s' was allocated but never freed before leaving scope.", ftp->name);
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
        current_function_has_return = 1;
        Node* expr = pop_tree();
        Node* retNode = new_ast_node("return");
        retNode->left = expr;
        retNode->right = NULL;
        push_tree(retNode);
    }
    | RETURN SEMICOLON {
        current_function_has_return = 1;
        Node* retNode = new_ast_node("return");
        retNode->left = NULL;
        retNode->right = NULL;
        push_tree(retNode);
    }
    ;

declaration
    : type_specifier init_declarator_list SEMICOLON
    | type_specifier init_declarator_list error { yyerror("Missing semicolon (;) after variable declaration."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    | error init_declarator_list SEMICOLON { yyerror("Missing valid type specifier (int, float, etc) for declaration."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    ;

statement
    : compound_statement
    | expression_statement
    | iteration_statement
    | condition_statement
    | error SEMICOLON { yyerror("Invalid statement; please check your syntax before the semicolon."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    | expression error { yyerror("Missing semicolon immediately following expression block."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    ;

condition_statement
    : IF LPAREN relational_expression RPAREN statement {
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *ifnode = new_ast_node("if");
        ifnode->left = cond;
        ifnode->right = stmt;
        push_tree(ifnode);
    }
    | IF error RPAREN statement { yyerror("Missing or malformed condition inside 'if'. Did you forget an opening '('?"); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    | IF LPAREN relational_expression error { yyerror("Missing closing ')' for 'if' condition."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    ;

iteration_statement
    : FOR LPAREN expression_statement expression_statement expression RPAREN statement {
        Node *stmt = pop_tree();
        Node *inc = pop_tree();
        Node *cond = pop_tree();
        Node *init = pop_tree();
        Node *fornode = new_ast_node("for");
        fornode->left = init;
        fornode->mid1 = cond;
        fornode->mid2 = inc;
        fornode->right = stmt;
        push_tree(fornode);
    }
    | FOR error statement { yyerror("Malformed 'for' loop declaration."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    | WHILE LPAREN relational_expression RPAREN statement {
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *whilenode = new_ast_node("while");
        whilenode->left = cond;
        whilenode->right = stmt;
        push_tree(whilenode);
    }
    | WHILE error RPAREN statement { yyerror("Missing or malformed condition inside 'while'. Did you forget an opening '('?"); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
    | WHILE LPAREN relational_expression error { yyerror("Missing closing ')' for 'while' condition."); create_node(AST_ERROR_TOKEN, 1); yyerrok; }
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
                error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "IMPLICIT_TRUNCATION", "Assignment kept for recovery", "Possible loss of data while assigning decimal value to integer '%s'.", id_ptr->name);
            } else if ((id_ptr->dtype == 0 || id_ptr->dtype == 1) && ($<ptr>4->dtype == 2)) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "TYPE_MISMATCH_INIT", "Keep parsing with error placeholder.", "Cannot initialize numeric variable '%s' with a character.", id_ptr->name);
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

            struct node* dummy = new_semantic_node(0, "dummy");
            dummy->val.i = 0;
            $$ = dummy;
            create_node("dummy", 1);
        } else {
            res->is_used = 1;

            struct node* ret = (struct node*)checked_calloc(1, sizeof(struct node));
            memcpy(ret, res, sizeof(struct node));
            $$ = ret;
            create_node(res->name, 1);
        }
    }
    | NUMBER {
        struct node* num_node = new_semantic_node(0, "__number__");
        num_node->val.i = $1;
        $$ = num_node;
        sprintf(tempStr, "%d", $1);
        create_node(tempStr, 1);
    }
    | FLOAT_VALUE {
        struct node* f_node = new_semantic_node(1, "__float__");
        f_node->val.f = $1;
        $$ = f_node;
        sprintf(tempStr, "%f", $1);
        create_node(tempStr, 1);
    }
    | STRING_LITERAL {
        struct node* s_node = new_semantic_node(4, "__string__");
        $$ = s_node;
        create_node("string_lit", 1);
    }
    | CHAR_LITERAL {
        struct node* c_node = new_semantic_node(2, "__char__");
        $$ = c_node;
        create_node("char_lit", 1);
    }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

argument_expression_list
    : assignment_expression {
        arg_list_info *info = (arg_list_info*)checked_calloc(1, sizeof(arg_list_info));
        info->count = 1;
        info->types[0] = $1 ? $1->dtype : -1;
        info->is_allocated[0] = ($1 ? $1->is_allocated : 0);
        snprintf(info->names[0], sizeof(info->names[0]), "%s", ($1 && $1->name[0]) ? $1->name : "");

        Node* expr = pop_tree();
        Node* paramNode = new_ast_node("param");
        paramNode->left = expr;
        paramNode->right = NULL;
        push_tree(paramNode);
        $$ = info;
    }
    | argument_expression_list COMMA assignment_expression {
        if ($1->count < MAX_PARAMS) {
            $1->types[$1->count] = $3 ? $3->dtype : -1;
            $1->is_allocated[$1->count] = ($3 ? $3->is_allocated : 0);
            snprintf($1->names[$1->count], sizeof($1->names[$1->count]), "%s", ($3 && $3->name[0]) ? $3->name : "");
            $1->count++;
        }
        Node* expr = pop_tree();
        Node* list = pop_tree();
        Node* paramNode = new_ast_node("param");
        paramNode->left = expr;
        paramNode->right = NULL;

        Node* seq = new_ast_node("param_list");
        seq->left = list;
        seq->right = paramNode;
        push_tree(seq);
        $$ = $1;
    }
    ;

postfix_expression
    : primary_expression
    | postfix_expression LBRACK expression RBRACK {
        struct node* arr_node = new_semantic_node(0, "__array_item__");
        $$ = arr_node;
        create_node("[]", 0);
    }
    | IDENTIFIER LPAREN RPAREN {
        if (strcmp($1, "gets") == 0) {
            error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "UNSAFE_API", "Call retained for analysis", "'%s' is unsafe and may lead to buffer overflows; use 'fgets' instead.", $1);
        }

        struct node* res = lookup($1);
        function_sig *sig = lookup_function($1);
        if (sig && sig->param_count != 0) {
            error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "ARG_COUNT_MISMATCH", "Match call arguments to function signature.", "Function '%s' expects %d argument(s), but 0 provided.", $1, sig->param_count);
            error_occurred = 1;
        }
        if(!res &&
           strcmp($1, "gets") != 0 && strcmp($1, "malloc") != 0 && strcmp($1, "free") != 0 &&
           strcmp($1, "printf") != 0 && strcmp($1, "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
        }

        Node* callNode = new_ast_node("call");

        Node* funcNode = new_ast_node($1);
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = NULL;
        push_tree(callNode);

        struct node* call_res = new_semantic_node(0, "__call__");
        if (strcmp($1, "malloc") == 0) strcpy(call_res->name, "__malloc__");
        $$ = call_res;
    }
    | IDENTIFIER LPAREN argument_expression_list RPAREN {
        if (strcmp($1, "gets") == 0) {
            error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "UNSAFE_API", "Call retained for analysis", "'%s' is unsafe and may lead to buffer overflows; use 'fgets' instead.", $1);
        }

        if (strcmp($1, "free") == 0) {
            if ($3 && $3->count == 1 && $3->is_allocated[0] == 1) {
                struct node* free_target = lookup($3->names[0]);
                if (free_target) free_target->is_allocated = 0;
            }
        }

        struct node* res = lookup($1);
        function_sig *sig = lookup_function($1);
        if (sig) {
            if (sig->param_count != $3->count) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "ARG_COUNT_MISMATCH", "Match call arguments to function signature.", "Function '%s' expects %d argument(s), but %d provided.", $1, sig->param_count, $3->count);
                error_occurred = 1;
            } else {
                for (int i = 0; i < sig->param_count; i++) {
                    if ($3->types[i] >= 0 && sig->param_types[i] != $3->types[i]) {
                        error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "ARG_TYPE_MISMATCH", "Adjust argument type or function parameter type.", "Argument %d of '%s' expects type %s, but got %s.", i + 1, $1, dtype_name(sig->param_types[i]), dtype_name($3->types[i]));
                        error_occurred = 1;
                    }
                }
            }
        }
        if(!res &&
           strcmp($1, "gets") != 0 && strcmp($1, "malloc") != 0 && strcmp($1, "free") != 0 &&
           strcmp($1, "printf") != 0 && strcmp($1, "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", $1);
            report_error(lno, errmsg);
        }
        Node* args = pop_tree();
        Node* callNode = new_ast_node("call");

        Node* funcNode = new_ast_node($1);
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = args;
        push_tree(callNode);

        struct node* call_res = new_semantic_node(0, "__call__");
        if (strcmp($1, "malloc") == 0) strcpy(call_res->name, "__malloc__");
        $$ = call_res;
        free($3);
    }
    ;

unary_expression
    : postfix_expression
    | BIT_AND unary_expression {
         struct node* res = new_semantic_node(4, "__addr__");
         $$ = res;

         Node* operand = pop_tree();
         Node* opNode = new_ast_node("addr");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
    | MULT unary_expression {
         struct node* res = new_semantic_node(4, "__deref__");
         $$ = res;

         Node* operand = pop_tree();
         Node* opNode = new_ast_node("deref");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression MULT unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform multiplication on strings or characters."); }
        $$ = new_semantic_node(0, "__mul__"); create_node("*", 0);
    }
    | multiplicative_expression DIV unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform division on strings or characters."); }
        float rhs = ($3->dtype == 0) ? (float)$3->val.i : $3->val.f;
        if(rhs == 0.0) {
            report_error(lno, "Division by zero");
        }
        $$ = new_semantic_node(0, "__div__"); create_node("/", 0);
    }
    | multiplicative_expression MOD unary_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot modulo strings or characters."); }
        $$ = new_semantic_node(0, "__mod__"); create_node("%", 0);
    }
    ;

additive_expression
    : multiplicative_expression
    | additive_expression PLUS multiplicative_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly add strings or characters."); }
        $$ = new_semantic_node(0, "__add__"); create_node("+", 0);
    }
    | additive_expression MINUS multiplicative_expression {
        if (($1->dtype == 4 || $1->dtype == 2) || ($3->dtype == 4 || $3->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly subtract strings or characters."); }
        $$ = new_semantic_node(0, "__sub__"); create_node("-", 0);
    }
    ;

relational_expression
    : additive_expression
    | relational_expression LE additive_expression { $$ = new_semantic_node(0, "__le__"); create_node("<=", 0); }
    | relational_expression GE additive_expression { $$ = new_semantic_node(0, "__ge__"); create_node(">=", 0); }
    | relational_expression LT additive_expression { $$ = new_semantic_node(0, "__lt__"); create_node("<", 0); }
    | relational_expression GT additive_expression { $$ = new_semantic_node(0, "__gt__"); create_node(">", 0); }
    | relational_expression EQ additive_expression { $$ = new_semantic_node(0, "__eq__"); create_node("==", 0); }
    | relational_expression NE additive_expression { $$ = new_semantic_node(0, "__ne__"); create_node("!=", 0); }
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

            struct node* dummy = new_semantic_node(0, "dummy");
            dummy->val.i = 0;
            $$ = dummy;
        } else {
            if (res->dtype == 0 && $4->dtype == 1) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_WARNING, lno, 0, "IMPLICIT_TRUNCATION", "Assignment kept for recovery", "Possible loss of data while assigning decimal value to integer '%s'.", res->name);
            } else if ((res->dtype == 0 || res->dtype == 1) && ($4->dtype == 2)) {
                error_logf(ERR_PHASE_SEM, ERR_SEV_ERROR, lno, 0, "TYPE_MISMATCH_ASSIGN", "Keep parsing with error placeholder.", "Cannot assign character to numeric variable '%s'.", res->name);
                error_occurred = 1;
            } else if ((res->dtype == 0 || res->dtype == 1) && $4->dtype == 4 && res->dtype != 4) {

            }
            if ($4 && strcmp($4->name, "__malloc__") == 0) {
                res->is_allocated = 1;
            }
            if ($4 && $4->is_allocated == 1 && strcmp($4->name, res->name) != 0) {

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
    struct node* dummy = new_semantic_node(0, "dummy");
    dummy->val.i = 0;
    $$ = dummy;
} | expression SEMICOLON { $$ = $1; };

%%
int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    jmp_buf fatal_env;
    error_reset_all();
    error_set_fatal_jmp(&fatal_env);
    if (setjmp(fatal_env) != 0) {
        fprintf(stderr, "[fatal][SYS] line=%d col=0 code=ABORTED: Compilation aborted due to fatal diagnostics.\n", lno);
        return EXIT_FAILURE;
    }

    printf("Starting modern C parser...\n");
    printf("\n--- Lexical Tokens ---\n");
    printf("LINE  TOKEN              TEXT\n");
    printf("====  =================  ====================\n");
    int parse_status = yyparse();
    if (parse_status != 0 && !error_has_errors()) {
        report_error(lno, "Parsing failed due to unrecoverable syntax errors.");
    }
    return parse_status;
}

static void* checked_calloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    if (!ptr) {
        error_log(ERR_PHASE_SYS, ERR_SEV_FATAL, lno, 0, "OOM_ALLOC", "Abort compilation.", "Out of memory while compiling.");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static struct node* new_semantic_node(int dtype, const char *name) {
    struct node* n = (struct node*)checked_calloc(1, sizeof(struct node));
    n->dtype = dtype;
    if (name && name[0] != '\0') {
        snprintf(n->name, sizeof(n->name), "%s", name);
    } else {
        snprintf(n->name, sizeof(n->name), "%s", "__tmp__");
    }
    return n;
}

static Node* new_ast_node(const char *token) {
    Node* n = (Node*)checked_calloc(1, sizeof(Node));
    if (token) {
        snprintf(n->token, sizeof(n->token), "%s", token);
    }
    return n;
}

static function_sig* lookup_function(const char *name) {
    function_sig *p = functions;
    while (p) {
        if (strcmp(p->name, name) == 0) return p;
        p = p->next;
    }
    return NULL;
}

static void upsert_function_signature(const char *name, int return_type, int param_count, int *param_types) {
    function_sig *sig = lookup_function(name);
    if (!sig) {
        sig = (function_sig*)checked_calloc(1, sizeof(function_sig));
        snprintf(sig->name, sizeof(sig->name), "%s", name);
        sig->next = functions;
        functions = sig;
    }
    sig->return_type = return_type;
    sig->param_count = (param_count > MAX_PARAMS) ? MAX_PARAMS : param_count;
    for (int i = 0; i < sig->param_count; i++) {
        sig->param_types[i] = param_types[i];
    }
}

static const char* dtype_name(int dtype) {
    switch (dtype) {
        case 0: return "int";
        case 1: return "float";
        case 2: return "char";
        case 3: return "void";
        case 4: return "pointer/string";
        default: return "unknown";
    }
}

void report_error(int line, const char* msg) {
    if(!error_occurred) error_occurred = 1;
    error_log(ERR_PHASE_SYN, ERR_SEV_ERROR, line, colno, "PARSER_ERROR", msg, "Panic mode recovery attempted.");
}

void yyerror(const char* s) {
    report_error(lno, (s && s[0] != '\0') ? s : "Syntax error");
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
    if (type == 3 && current_insert_kind != SYM_KIND_FUNC) {
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
    struct node *newnode = (struct node*)checked_calloc(1, sizeof(struct node));
    snprintf(newnode->name, sizeof(newnode->name), "%s", name);
    newnode->dtype = type;
    newnode->scope = scope;
    newnode->valid = 1;
    newnode->is_used = 0;
    newnode->sym_kind = current_insert_kind;
    newnode->is_allocated = 0;
    newnode->link = first;
    first = newnode;
    return newnode;
}

static const char* sym_kind_name(int k) {
    switch (k) {
        case SYM_KIND_VAR:   return "variable";
        case SYM_KIND_FUNC:  return "function";
        case SYM_KIND_PARAM: return "parameter";
        default:             return "unknown";
    }
}

static const char* sym_type_name(int t) {
    switch (t) {
        case 0: return "int";
        case 1: return "float";
        case 2: return "char";
        case 3: return "void";
        case 4: return "pointer";
        default: return "unknown";
    }
}

void printsymtable() {
    printf("\n--- Symbol Table ---\n");
    printf("NAME            KIND        TYPE      SCOPE   STATUS\n");
    printf("==============  ==========  ========  ======  ======\n");
    struct node *ptr = first;
    while(ptr != NULL) {
        if (!(ptr->sym_kind == SYM_KIND_FUNC && strcmp(ptr->name, "main") == 0)) {
            printf("%-14s  %-10s  %-8s  %-6d  %s\n",
                   ptr->name,
                   sym_kind_name(ptr->sym_kind),
                   sym_type_name(ptr->dtype),
                   ptr->scope,
                   ptr->valid ? "active" : "dead");
        }
        ptr = ptr->link;
    }

    printf("\n--- Symbol Table JSON ---\n[");
    int first_row = 1;
    ptr = first;
    while(ptr != NULL) {
        if (!(ptr->sym_kind == SYM_KIND_FUNC && strcmp(ptr->name, "main") == 0)) {
            if (!first_row) fputc(',', stdout);
            first_row = 0;
            printf("{\"name\":\"%s\",\"kind\":\"%s\",\"type\":\"%s\",\"scope\":%d,\"status\":\"%s\"}",
                   ptr->name,
                   sym_kind_name(ptr->sym_kind),
                   sym_type_name(ptr->dtype),
                   ptr->scope,
                   ptr->valid ? "active" : "dead");
        }
        ptr = ptr->link;
    }
    printf("]\n");
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
    if (!newnode) return;
    tree_stack *temp = (tree_stack*)checked_calloc(1, sizeof(tree_stack));
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
    Node *newnode = new_ast_node(token);
    if (leaf) {
        newnode->left = newnode->right = NULL;
    } else {
        newnode->right = pop_tree();
        newnode->left = pop_tree();
        if (!newnode->left) newnode->left = new_ast_node(AST_ERROR_TOKEN);
        if (!newnode->right) newnode->right = new_ast_node(AST_ERROR_TOKEN);
    }

    newnode->eval_type = tree_top && tree_top->node ? tree_top->node->eval_type : 0;
    push_tree(newnode);
}

static CSTNode *cst_new(const char *label, int is_leaf) {
    CSTNode *n = (CSTNode*)checked_calloc(1, sizeof(CSTNode));
    snprintf(n->label, sizeof(n->label), "%s", label);
    n->is_leaf = is_leaf;
    return n;
}

static void cst_add(CSTNode *parent, CSTNode *child) {
    if (!parent || !child) return;
    if (parent->n_children == parent->cap) {
        int nc = parent->cap ? parent->cap * 2 : 4;
        parent->children = (CSTNode**)realloc(parent->children, nc * sizeof(CSTNode*));
        parent->cap = nc;
    }
    parent->children[parent->n_children++] = child;
}

static void cst_free(CSTNode *n) {
    if (!n) return;
    for (int i = 0; i < n->n_children; i++) cst_free(n->children[i]);
    free(n->children);
    free(n);
}

static void cst_print(CSTNode *n, const char *prefix, int is_last, int is_root) {
    if (!n) return;
    if (is_root) {
        printf("%s\n", n->label);
    } else {
        printf("%s%s %s\n", prefix, is_last ? "`--" : "+--", n->label);
    }
    char child_prefix[1024];
    if (is_root) {
        child_prefix[0] = '\0';
    } else {
        snprintf(child_prefix, sizeof(child_prefix), "%s%s",
                 prefix, is_last ? "   " : "|  ");
    }
    for (int i = 0; i < n->n_children; i++) {
        cst_print(n->children[i], child_prefix, i == n->n_children - 1, 0);
    }
}

static void cst_print_json(CSTNode *n, FILE *out) {
    if (!n) { fputs("null", out); return; }
    fputs("{\"label\":\"", out);
    for (const char *p = n->label; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c == '"' || c == '\\') { fputc('\\', out); fputc(c, out); }
        else if (c == '\n') fputs("\\n", out);
        else if (c == '\r') fputs("\\r", out);
        else if (c == '\t') fputs("\\t", out);
        else if (c < 0x20) fprintf(out, "\\u%04x", c);
        else fputc(c, out);
    }
    fputs("\",\"children\":[", out);
    for (int i = 0; i < n->n_children; i++) {
        if (i) fputc(',', out);
        cst_print_json(n->children[i], out);
    }
    fputs("]}", out);
}

static int cst_pos = 0;

static int tk_is(int idx, const char *type) {
    return idx < token_buf_count && strcmp(token_buf[idx].type, type) == 0;
}

static int tk_is_type_kw(int idx) {
    if (idx >= token_buf_count) return 0;
    const char *t = token_buf[idx].type;
    return !strcmp(t,"INT") || !strcmp(t,"FLOAT") || !strcmp(t,"CHAR") ||
           !strcmp(t,"VOID") || !strcmp(t,"DOUBLE") || !strcmp(t,"LONG") ||
           !strcmp(t,"SHORT") || !strcmp(t,"UNSIGNED") || !strcmp(t,"SIGNED") ||
           !strcmp(t,"CONST") || !strcmp(t,"STATIC") || !strcmp(t,"EXTERN");
}

static CSTNode *cst_consume(void) {
    if (cst_pos >= token_buf_count) return NULL;
    CSTNode *n = cst_new(token_buf[cst_pos].text, 1);
    cst_pos++;
    return n;
}

static CSTNode *cst_parse_statement(void);
static CSTNode *cst_parse_block(void);

static CSTNode *cst_parse_paren_group(const char *label) {
    CSTNode *n = cst_new(label, 0);
    int depth = 0;
    if (tk_is(cst_pos, "LPAREN")) { cst_add(n, cst_consume()); depth++; }
    while (cst_pos < token_buf_count && depth > 0) {
        if (tk_is(cst_pos, "LPAREN")) depth++;
        else if (tk_is(cst_pos, "RPAREN")) depth--;
        cst_add(n, cst_consume());
    }
    return n;
}

static CSTNode *cst_parse_until_semi(const char *label) {
    CSTNode *n = cst_new(label, 0);
    int depth = 0;
    while (cst_pos < token_buf_count) {
        if (tk_is(cst_pos, "LPAREN") || tk_is(cst_pos, "LBRACK")) depth++;
        else if (tk_is(cst_pos, "RPAREN") || tk_is(cst_pos, "RBRACK")) depth--;
        int is_semi = tk_is(cst_pos, "SEMICOLON");
        cst_add(n, cst_consume());
        if (depth <= 0 && is_semi) break;
        if (depth <= 0 && tk_is(cst_pos, "RBRACE")) break;
    }
    return n;
}

static CSTNode *cst_parse_block(void) {
    CSTNode *n = cst_new("block", 0);
    if (tk_is(cst_pos, "LBRACE")) cst_add(n, cst_consume());
    while (cst_pos < token_buf_count && !tk_is(cst_pos, "RBRACE")) {
        cst_add(n, cst_parse_statement());
    }
    if (tk_is(cst_pos, "RBRACE")) cst_add(n, cst_consume());
    return n;
}

static CSTNode *cst_parse_if(void) {
    CSTNode *n = cst_new("if_statement", 0);
    cst_add(n, cst_consume());
    if (tk_is(cst_pos, "LPAREN")) cst_add(n, cst_parse_paren_group("condition"));
    cst_add(n, cst_parse_statement());
    if (tk_is(cst_pos, "ELSE")) {
        cst_add(n, cst_consume());
        cst_add(n, cst_parse_statement());
    }
    return n;
}

static CSTNode *cst_parse_while(void) {
    CSTNode *n = cst_new("while_statement", 0);
    cst_add(n, cst_consume());
    if (tk_is(cst_pos, "LPAREN")) cst_add(n, cst_parse_paren_group("condition"));
    cst_add(n, cst_parse_statement());
    return n;
}

static CSTNode *cst_parse_for(void) {
    CSTNode *n = cst_new("for_statement", 0);
    cst_add(n, cst_consume());
    if (tk_is(cst_pos, "LPAREN")) cst_add(n, cst_parse_paren_group("for_header"));
    cst_add(n, cst_parse_statement());
    return n;
}

static CSTNode *cst_parse_statement(void) {
    if (cst_pos >= token_buf_count) return cst_new("(empty)", 0);
    if (tk_is(cst_pos, "IF"))     return cst_parse_if();
    if (tk_is(cst_pos, "WHILE"))  return cst_parse_while();
    if (tk_is(cst_pos, "FOR"))    return cst_parse_for();
    if (tk_is(cst_pos, "LBRACE")) return cst_parse_block();
    if (tk_is(cst_pos, "RETURN")) return cst_parse_until_semi("return_statement");
    if (tk_is_type_kw(cst_pos))   return cst_parse_until_semi("declaration");
    return cst_parse_until_semi("expression_statement");
}

static CSTNode *cst_parse_function_def(void) {
    CSTNode *n = cst_new("function_definition", 0);
    cst_add(n, cst_consume());
    while (tk_is(cst_pos, "MULT")) cst_add(n, cst_consume());
    if (tk_is(cst_pos, "IDENTIFIER")) cst_add(n, cst_consume());
    if (tk_is(cst_pos, "LPAREN")) cst_add(n, cst_parse_paren_group("parameters"));
    if (tk_is(cst_pos, "LBRACE")) cst_add(n, cst_parse_block());
    return n;
}

static CSTNode *cst_parse_program(void) {
    cst_pos = 0;
    CSTNode *root = cst_new("program", 0);
    while (cst_pos < token_buf_count) {
        if (tk_is(cst_pos, "PREPROC")) {
            CSTNode *p = cst_new("preprocessor", 0);
            cst_add(p, cst_consume());
            cst_add(root, p);
            continue;
        }
        if (tk_is_type_kw(cst_pos)) {
            int p = cst_pos + 1;
            while (p < token_buf_count && (tk_is(p, "MULT") || tk_is(p, "IDENTIFIER"))) p++;
            if (p < token_buf_count && tk_is(p, "LPAREN")) {
                cst_add(root, cst_parse_function_def());
            } else {
                cst_add(root, cst_parse_until_semi("declaration"));
            }
            continue;
        }
        if (tk_is(cst_pos, "LBRACE")) { cst_add(root, cst_parse_block()); continue; }
        cst_add(root, cst_parse_statement());
    }
    return root;
}

void clear_tree(Node* root) {
    if (root == NULL) return;
    clear_tree(root->left);
    clear_tree(root->mid1);
    clear_tree(root->mid2);
    clear_tree(root->right);
    free(root);
}

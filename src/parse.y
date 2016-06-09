%{
/* bison -y -oparse.c parse.y */
#define YYERROR_VERBOSE 1
#define YYDEBUG 1
#define YYENABLE_NLS 1
#define YYLEX_PARAM &yylval, &yylloc

#include <stddef.h>

#include "latino.h"
#include "ast.h"
#include "lex.h"

#ifdef __linux
#include <libintl.h>
#define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#endif

int yyerror(struct YYLTYPE *yylloc_param, void *scanner, struct ast **root, const char *s);
int yylex (YYSTYPE * yylval_param,YYLTYPE * yylloc_param ,yyscan_t yyscanner);

%}

%output "parse.c"
%defines "parse.h"

%locations
%define api.pure
%lex-param {void *scanner}
%parse-param {ast **root}
%parse-param {void *scanner}

/* declare tokens */
%token <node> TINT
%token <node> TSTRING
%token <node> TIDENTIFIER
%token
    KIF
    KEND
    KELSE

%token
    OP_EQ

%type <node> program statement_list
%type <node> declaration
%type <node> equality_expression expression constant_expression
%type <node> statement function_call argument_expression_list
%type <node> primary_expression
%type <node> selection_statement

/*
 * precedencia de operadores
 * 0: -
 * 1: * /
 * 2: + -
 *
 */
%right '='
%left OP_EQ

%start program

%%

primary_expression:
      TIDENTIFIER { $$ = $1; }
    | constant_expression  { $$ = $1; }
    ;

constant_expression:
      TINT { $$ = $1; }
    | TSTRING { $$ = $1; }
    ;

equality_expression:
    expression OP_EQ expression { $$ = nodo_nuevo(NODO_IGUALDAD, $1, $3); }
    ;

program
    : statement_list {
        *root = $1;
    }
    ;

statement_list
    : statement_list statement {
        if($2){
            $$ = nodo_nuevo(NODO_BLOQUE, $2, $1);
        }
    }
    | statement {
        $$ = nodo_nuevo(NODO_BLOQUE, $1, NULL);
    }
    ;


statement: /* empty */ { $$ = NULL; }
    | selection_statement { $$ = $1; }
    | expression { $$ = $1; }
    | declaration { $$ = $1; }
    | function_call { $$ = $1; }
    ;

declaration:
      TIDENTIFIER '=' expression { $$ = nodo_nuevo_asignacion($3, $1); }
    ;

selection_statement:
    KIF expression statement_list KEND {
        $$ = nodo_nuevo_si($2, $3, NULL); }
    | KIF expression statement_list KELSE statement_list KEND {
        $$ = nodo_nuevo_si($2, $3, $5); }
    ;

function_call:
     TIDENTIFIER '(' argument_expression_list ')' { $$ = nodo_nuevo(NODO_FUNCION_LLAMADA, $1, $3); }
    ;

argument_expression_list: /* empty */ { $$ = NULL; }
    | expression { $$ = nodo_nuevo(NODO_FUNCION_ARGUMENTOS, $1, NULL); }
    | expression ',' argument_expression_list { $$ = nodo_nuevo(NODO_FUNCION_ARGUMENTOS, $1, $3); }
    ;

expression
    : '(' expression ')' { $$ = $2; }
    | equality_expression
    | primary_expression
    | function_call
    ;

%%

//se define para analisis sintactico (bison)
int yyerror(struct YYLTYPE *yylloc_param, void *scanner, struct ast **root,
            const char *s) {
  if(!analisis_silencioso){
  lat_registrar_error("Linea %d: %s", (yylloc_param->first_line + 1), s);
  }
  return 0;
}

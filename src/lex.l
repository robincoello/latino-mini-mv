%option nounistd
%option outfile="lex.c" header-file="lex.h"
%option warn nodefault
%option reentrant noyywrap never-interactive
%option bison-bridge bison-locations
%option yylineno
%option extra-type="struct lex_state *"
%option nounput
%option noinput

%{

/* flex -olex.c -i lex.l */
#include <string.h>
#include <stdlib.h>

#include "latino.h"
#include "ast.h"
#include "parse.h"

#define YYBISON 1

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

int f(int token, yyscan_t scanner);

#if YYBISON
union YYSTYPE;
extern int yylex (YYSTYPE * yylval_param,YYLTYPE * yylloc_param ,yyscan_t yyscanner);
#endif

%}

%%

 /* operadores de un solo caracter */
"(" |
")" |
"="     { return yytext[0]; }

 /* operadores de comparacion */
"=="    { return OP_EQ; }

 /* palabras reservadas */
"si"        { return KIF; }
"fin"       { return KEND; }
"sino"      { return KELSE; }

 /* names */
[_a-zA-Z][_a-zA-Z0-9]* { yylval->node = nodo_nuevo_identificador(yytext, yylloc->first_line, yylloc->first_column); return TIDENTIFIER; }
[-+]?[0-9]+                 { yylval->node = nodo_nuevo_entero(strtol(yytext, NULL, 0), yylloc->first_line, yylloc->first_column); return TINT; }
\"([^\\\"]|\\.)*\"     { yytext[strlen(yytext) - 1] = '\0'; yylval->node = nodo_nuevo_cadena(yytext+1, yylloc->first_line, yylloc->first_column); return TSTRING; }
"#".* /* ignore comments */
[\n]                   { yycolumn = 1; }
[ \t]+                 { ; } /* ignora espacios en blanco y tabuladores */
.                      { ; } /* ignora caracteres incorrectos */

%%

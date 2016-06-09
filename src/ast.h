/*
The MIT License (MIT)

Copyright (c) 2015 - Latino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef _AST_H_
#define _AST_H_

#include <stdbool.h>
#include "vm.h"

/** \file ast.h
*
* Contiene las estructuras y metodos necesarios para el manejo del analizador lexico sintactico
*
*/

/** \brief Tipos de dato */
typedef enum {
  VALOR_NULO, /**< Valor nulo */
  VALOR_LOGICO, /**< Valor logico */
  VALOR_ENTERO, /**< Valor entero */
  VALOR_CADENA /**< Valor cadena */
} nodo_tipo_valor;

/** \brief Valores del dato */
typedef struct {
  nodo_tipo_valor t; /**< Nodo tipo valor */
  bool es_constante; /**< Para validar si es constante */
  int num_linea; /**< Numero de linea */
  int num_columna; /**< Numero de columna */
  /** Contiene los valores del dato*/
  union val {
    int b;  /**< Logico */
    long i; /**< Entero */
    double d;   /**< Decimal */
    char *c;    /**< Literal */
    char *s;    /**< Cadena */
    void *f;    /**< Funcion */
  } v;
} nodo_valor;

/** \brief Tipos de nodos en arbol abstracto de sintaxis (abstract syntax tree) */
typedef enum {
  NODO_IGUALDAD,  /**< Nodo igualdad */
  NODO_BLOQUE,  /**< Nodo bloque */
  NODO_SI,  /**< Nodo si */
  NODO_IDENTIFICADOR,  /**< Nodo identificador */
  NODO_ENTERO,  /**< Nodo entero */
  NODO_CADENA,  /**< Nodo cadena */
  NODO_RETORNO,
  NODO_FUNCION_ARGUMENTOS,
  NODO_FUNCION_USUARIO,
  NODO_ASIGNACION,
  NODO_LISTA_PARAMETROS,
  NODO_FUNCION_LLAMADA,
} nodo_tipo;

/** \brief Nodos en arbol abstacto de sintaxis (Abstract Syntax Tree).
  *
  * Todos los nodos son inicializados con un tipo de nodo */
typedef struct ast {
  nodo_tipo tipo;   /**< Tipo de nodo */
  nodo_valor *valor; /**< Valor del nodo */
  struct ast *l; /**< Nodo izquierdo */
  struct ast *r; /**< Nodo derecho */
  lat_mv *mv;
} ast;

/** \brief Estado del analizador lexico */
typedef struct lex_state { int insert; } lex_state;

/** \brief Tipo de dato que se envia al analizador lexico */
typedef union YYSTYPE {
  int token;
  ast *node;
} YYSTYPE;

/** \brief nodo para representar un ast SI (if).
  *
  * si (condicion)
  *     [sentencias]
  * sino
  *     [sentencias]
  * fin
  */
typedef struct {
  nodo_tipo tipo;
  struct ast *condicion; /**< Condicion */
  struct ast *entonces;   /**< Instrucciones que se ejecutan si la condicion es verdadera */
  struct ast *sino;   /**< Instrucciones que se ejecutan si la condicion es falsa */
} nodo_si;

/** \brief nodo para representar una funcion.
  *
  * funcion nombre_fun ([param1, param2, ... ])
  *     [sentencias]
  * fin
  */
typedef struct {
  nodo_tipo tipo;
  struct ast *nombre;
  struct ast *parametros;
  struct ast *sentencias;
} nodo_funcion;

/** \brief Nuevo nodo generico para el AST
  *
  * \param tipo: Tipo de nodo
  * \param l: Nodo izquierdo
  * \param r: Nodo derecho
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo(nodo_tipo tipo, ast *l, ast *r);

/** Nuevo nodo tipo Identificador (var)
  *
  * \param s: nombre del identificador
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo_identificador(const char *s, int num_linea, int num_columna);

/** Nuevo nodo tipo Entero (1234)
  *
  * \param i: Nodo valor
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo_entero(long i, int num_linea, int num_columna);

/** Nuevo nodo tipo Cadena ("Esto es una 'cadena'")
  *
  * \param s: Nodo valor
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo_cadena(const char *s, int num_linea, int num_columna);

/** Nuevo nodo tipo Operador (var1 + var2)
  *
  * \param nt: tipo de nodo
  * \param l: Nodo izquierdo
  * \param r: Nodo derecho
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo(nodo_tipo nt, ast *l, ast *r);

/** Nuevo nodo tipo Asignacion (var = "hola latino")
  *
  * \param s: Nodo identificador
  * \param v: Nodo valor
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo_asignacion(ast *s, ast *v);

/** Nuevo nodo tipo si (if)
  *
  * \param cond: Nodo condicion ( a > b && a > c )
  * \param th: Nodo lista de sentencias (en caso de ser verdadera la condicion)
  * \param el: Nodo lista de sentencias (en caso de ser falsa la condicion)
  * \return ast: Un nodo AST
  *
  */
ast *nodo_nuevo_si(ast *cond, ast *th, ast *el);

/** Libera la memoria creada dinamicamente
  *
  * \param a: Nodo AST
  *
  */
void nodo_liberar(ast *a);

/** Analiza el arbol abstracto de sintaxis
  *
  * \param vm: Referencia a un objeto tipo maquina virtual
  * \param tree: Arbol abstracto de sintaxis
  * \return lat_object: objeto generico
  *
  */
lat_objeto *nodo_analizar_arbol(lat_mv *mv, ast *tree);

/** Analiza un nodo del arbol abstracto de sintaxis
  *
  * \param vm: Referencia a un objeto tipo maquina virtual
  * \param node: Nodo AST
  * \param bcode: Referencia a un objeto tipo bytecode
  * \param i: numero de instruccion actual
  * \return int: numero de instruccion siguiente
  *
  */
int nodo_analizar(lat_mv *mv, ast *node, lat_bytecode *bcode, int i);

#endif /*_AST_H_*/

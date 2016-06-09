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
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "vm.h"
#include "utils.h"
#include "libmem.h"
#include "libstring.h"

lat_mv* lat_crear_maquina_virtual()
{
    lat_mv* ret = (lat_mv*)lat_asignar_memoria(sizeof(lat_mv));
    ret->pila = lat_crear_lista();
    ret->todos_objetos = lat_crear_lista();
    ret->basurero_objetos = lat_crear_lista();
    ret->modulos = lat_crear_lista();
    ret->memoria_usada = 0;
    ret->objeto_cierto = lat_logico_nuevo(ret, true);
    ret->objeto_falso = lat_logico_nuevo(ret, false);
    //memset(ret->registros, 0, 256);
    memset(ret->contexto_pila, 0, 256);
    ret->contexto_pila[0] = lat_instancia(ret);
    ret->apuntador_pila = 0;
    lat_asignar_contexto_objeto(lat_obtener_contexto(ret), lat_cadena_nueva(ret, "=="), lat_definir_cfuncion(ret, lat_igualdad));
    lat_asignar_contexto_objeto(lat_obtener_contexto(ret), lat_cadena_nueva(ret, "imprimir"), lat_definir_cfuncion(ret, lat_imprimir));
    lat_asignar_contexto_objeto(lat_obtener_contexto(ret), lat_cadena_nueva(ret, "escribir"), lat_definir_cfuncion(ret, lat_imprimir));
    lat_asignar_contexto_objeto(lat_obtener_contexto(ret), lat_cadena_nueva(ret, "salir"), lat_definir_cfuncion(ret, lat_salir));
    return ret;
}

void lat_apilar(lat_mv *mv, lat_objeto* o)
{
    insert_list(mv->pila, (void*)o);
}

lat_objeto* lat_desapilar(lat_mv *mv)
{
    list_node* n = mv->pila->next;
    if (n->data == NULL)
    {
        lat_registrar_error("Pila vacia");
    }
    else
    {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        lat_objeto* ret = (lat_objeto*)n->data;
        lat_liberar_memoria(n);
        return ret;
    }
    return NULL;
}

void lat_apilar_lista(lat_objeto* lista, lat_objeto* o)
{
    insert_list(lista->datos.lista, (void*)o);
}

lat_objeto* lat_desapilar_lista(lat_objeto* lista)
{
    list_node* n = ((list_node*)lista)->next;
    if (n->data == NULL)
    {
        lat_registrar_error("Lista vacia");
    }
    else
    {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        lat_objeto* ret = (lat_objeto*)n->data;
        return ret;
    }
    return NULL;
}

void lat_apilar_contexto(lat_mv *mv)
{
    if (mv->apuntador_pila >= MAX_STACK_SIZE)
    {
        lat_registrar_error("Namespace desborde de la pila");
    }
    mv->contexto_pila[mv->apuntador_pila + 1] = lat_clonar_objeto(mv, mv->contexto_pila[mv->apuntador_pila]);
    mv->apuntador_pila++;
}

void lat_desapilar_contexto(lat_mv *mv)
{
    if (mv->apuntador_pila == 0)
    {
        lat_registrar_error("Namespace pila vacia");
    }
    lat_eliminar_objeto(mv, mv->contexto_pila[mv->apuntador_pila--]);
}

void lat_apilar_contexto_predefinido(lat_mv *mv, lat_objeto* ctx)
{
    if (mv->apuntador_pila >= 255)
    {
        lat_registrar_error("Namespace desborde de la pila");
    }
    mv->contexto_pila[++mv->apuntador_pila] = ctx;
}

lat_objeto* lat_desapilar_contexto_predefinido(lat_mv *mv)
{
    if (mv->apuntador_pila == 0)
    {
        lat_registrar_error("Namespace pila vacia");
    }
    return mv->contexto_pila[mv->apuntador_pila--];
}

lat_objeto* lat_obtener_contexto(lat_mv *mv)
{
    return mv->contexto_pila[mv->apuntador_pila];
}

lat_objeto* lat_definir_funcion(lat_mv *mv, lat_bytecode* inslist, int num_params)
{
    lat_objeto* ret = lat_funcion_nueva(mv);
    lat_funcion* fval = (lat_funcion*)lat_asignar_memoria(sizeof(lat_funcion));
    fval->bcode = inslist;
    fval->num_params = num_params;
    ret->datos.funcion = fval;
    //mv->memoria_usada += sizeof(sizeof(lat_function));
    return ret;
}

lat_objeto* lat_definir_cfuncion(lat_mv *mv, void (*function)(lat_mv *mv))
{
    lat_objeto* ret = lat_cfuncion_nueva(mv);
    ret->datos.cfunc = function;
    return ret;
}

void lat_imprimir(lat_mv *mv)
{
    lat_objeto* in = lat_desapilar(mv);
    if (in->tipo == T_NULO)
    {
        fprintf(stdout, "%s\n", "nulo");
    }
    else if (in->tipo == T_INSTANCE)
    {
        fprintf(stdout, "%s\n", "Objeto");
    }
    else if (in->tipo == T_INT)
    {
        fprintf(stdout, "%ld\n", lat_obtener_entero(in));
    }
    else if (in->tipo == T_STR)
    {
        fprintf(stdout, "%s\n", lat_obtener_cadena(in));
    }
    else if (in->tipo == T_BOOL)
    {
        if (lat_obtener_logico(in))
        {
            fprintf(stdout, "%s\n", "verdadero");
        }
        else
        {
            fprintf(stdout, "%s\n", "falso");
        }
    }
    else if (in->tipo == T_FUNC)
    {
        fprintf(stdout, "%s\n", "Funcion");
    }
    else if (in->tipo == T_CFUNC)
    {
        fprintf(stdout, "%s\n", "C_Funcion");
    }
    else
    {
        fprintf(stdout, "Tipo desconocido %d\n", in->tipo);
    }
    lat_apilar(mv, in);
}

void lat_clonar(lat_mv *mv)
{
    lat_objeto* ns = lat_desapilar(mv);
    lat_apilar(mv, lat_clonar_objeto(mv, ns));
}

void lat_igualdad(lat_mv *mv)
{
    lat_objeto* b = lat_desapilar(mv);
    lat_objeto* a = lat_desapilar(mv);
    if (a->tipo == T_BOOL && b->tipo == T_BOOL)
    {
        lat_apilar(mv, lat_obtener_logico(a) == lat_obtener_logico(b) ? mv->objeto_cierto : mv->objeto_falso);
        return;
    }
    if ((a->tipo == T_INT) && (b->tipo == T_INT ))
    {
        lat_apilar(mv, (lat_obtener_entero(a) == lat_obtener_entero(b)) ? mv->objeto_cierto : mv->objeto_falso);
        return;
    }
    lat_apilar(mv, mv->objeto_falso);
}

lat_bytecode lat_bc(lat_ins i, void* a, void* b, void* c)
{
    lat_bytecode ret;
    ret.ins = i;
    ret.a = a;
    ret.b = b;
    ret.c = c;
    return ret;
}

void lista_modificar_elemento(list_node* l, void* data, int pos)
{
    list_node* c;
    int i = -1;
    if(pos < 0 || pos >= length_list(l))
    {
        lat_registrar_error("Indice fuera de rango");
    }
    for (c = l; c->next != NULL; c = c->next)
    {
        if(i == pos)
        {
            c->data = data;
            return;
        }
        i++;
    }
}

lat_objeto* lista_obtener_elemento(list_node* l, int pos)
{
    list_node* c;
    int i = -1;
    if(pos < 0 || pos >= length_list(l))
    {
        lat_registrar_error("Indice fuera de rango");
    }
    for (c = l; c->next != NULL; c = c->next)
    {
        if(i == pos)
        {
            return (lat_objeto *)c->data;
        }
        i++;
    }
    return NULL;
}

lat_objeto* lat_llamar_funcion(lat_mv *mv, lat_objeto* func)
{
    if (func->tipo == T_FUNC)
    {
        if(!mv->REPL)
        {
            lat_apilar_contexto(mv);
        }
        lat_asignar_contexto_objeto(lat_obtener_contexto(mv), lat_cadena_nueva(mv, "$"), func);
        lat_bytecode* inslist = ((lat_funcion*)func->datos.funcion)->bcode;
        lat_bytecode cur;
        int pos;
        for (pos = 0, cur = inslist[pos]; cur.ins != RETURN_VALUE; cur = inslist[++pos])
        {
            //printf("%i\t", pos);
            switch ((int)cur.ins)
            {
            /* redefinicion de instrucciones estilo Python*/
            case LOAD_CONST:
            {
                //lat_imprimir_lista(mv, mv->pila);
                lat_objeto *variable = (lat_objeto*)cur.a;
                lat_apilar(mv, variable);
                /*if(variable->tipo == T_STR){
                    printf("LOAD_CONST %s\n", variable->datos.cadena);
                }
                else{
                    printf("LOAD_CONST %ld\n", variable->datos.entero);
                }*/
            }
                break;
            case STORE_NAME:{
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto *contexto = lat_obtener_contexto(mv);
                    lat_objeto *variable = (lat_objeto*)cur.a;
                    lat_objeto *valor = lat_desapilar(mv);
                    lat_asignar_contexto_objeto(contexto, variable, valor);
                    //printf("STORE_NAME %s\n", variable->datos.cadena);
                }
                break;
            case LOAD_NAME: {
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto *contexto = lat_obtener_contexto(mv);
                    lat_objeto *variable = (lat_objeto*)cur.a;
                    lat_objeto *valor = lat_obtener_contexto_objeto(contexto, variable);
                    lat_apilar(mv, valor);
                    //printf("LOAD_NAME %s\n", variable->datos.cadena);
                }
                break;
            case COMPARE_OP_EQ:
                //printf("COMPARE_OP_EQ\n");
                lat_igualdad(mv);
                break;
            case NOP:
                //printf("NOP\n");
                break;
            case JUMP_FORWARD:
                pos = ((int)cur.a - 1);
                //printf("JUMP_FORWARD %i\n", (int)cur.a);
                break;
            case POP_JUMP_IF_FALSE:
                {
                    //printf("POP_JUMP_IF_FALSE\n");
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto* cond = lat_desapilar(mv);
                    if(lat_obtener_logico(cond) == false){
                        pos = ((int)cur.a - 1);
                    }
                    //printf("POP_JUMP_IF_FALSE\t%i\n", (int)cur.a);
                }
                break;
            case POP_JUMP_IF_TRUE:
                {
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto* cond = lat_desapilar(mv);
                    if(lat_obtener_logico(cond) == true){
                        pos = ((int)cur.a - 1);
                    }
                    //printf("POP_JUMP_IF_TRUE\t%i\n", (int)cur.a);
                }
                break;
            case MAKE_FUNCTION: {
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto* funcion_usuario = lat_definir_funcion(mv, (lat_bytecode*)cur.a, (int)cur.b);
                    lat_apilar(mv, funcion_usuario);
                    //printf("MAKE_FUNCTION\n");
                }
                break;
            case CALL_FUNCTION:
                {
                    //lat_imprimir_lista(mv, mv->pila);
                    lat_objeto* funcion = lat_desapilar(mv);
                    lat_objeto* resultado = NULL;
                    if(funcion->tipo == T_FUNC){
                        resultado = lat_llamar_funcion(mv, funcion);
                        if(resultado == NULL) {
                            resultado = lat_desapilar(mv);
                        }
                    }else{
                        lat_llamar_funcion(mv, funcion);
                    }
                    //printf("CALL_FUNCTION\n");
                }
                break;
            case RETURN_VALUE:
                {
                    //printf("RETURN_VALUE\t");
                    return NULL;
                }
                break;

            }   //end switch
        }   //end for
        if(!mv->REPL)
        {
            lat_desapilar_contexto(mv);
        }
    }
    else if (func->tipo == T_CFUNC)
    {
        ((void (*)(lat_mv*))(func->datos.funcion))(mv);
    }
    else
    {
        debug("func->type: %d", func->tipo);
        lat_registrar_error("Object not a function");
    }

    return NULL;
}

void lat_salir(lat_mv *mv)
{
    lat_apilar(mv, lat_entero_nuevo(mv, 0L));
    exit(0);
}

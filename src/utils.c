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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "latino.h"
#include "ast.h"
#include "utils.h"
#include "libmem.h"

char* strdup0(const char* s)
{
    size_t len = strlen(s);
    char* p;
    p = lat_asignar_memoria(len + 1);
    if (p)
    {
        strncpy(p, s, len);
    }
    p[len] = '\0';
    return p;
}

char* parse_string(const char* s, size_t len)
{
    char* ret = lat_asignar_memoria(len + 1);
    int i = 0;
    int j = 0;
    int c = '@';
    for (i = 0; i < len; i++)
    {
        switch (s[i])
        {
        case '\\':
        {
            switch (s[i + 1])
            {
            case 'a':
                c = '\n';
                i++;
                goto save;
            case 'b':
                c = '\n';
                i++;
                goto save;
            case 'f':
                c = '\n';
                i++;
                goto save;
            case 'n':
                c = '\n';
                i++;
                goto save;
            case 'r':
                c = '\n';
                i++;
                goto save;
            case 't':
                c = '\t';
                i++;
                goto save;
            case 'v':
                c = '\n';
                i++;
                goto save;
            default:
                break;
            }
        }
        break;
        default:
            c = s[i];
            break;
        }
save:
        ret[j] = c;
        j++;
    }
    ret[j] = '\0';
    return ret;
}


char* concat(char* s1, char* s2)
{
    char* s3 = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(s3, s1);
    strcat(s3, s2);
    return s3;
}

char *replace(char *str, char *orig, char *rep)
{
    char *buffer = lat_asignar_memoria(MAX_STR_LENGTH);
    char *p;
    if (!(p = strstr(str, orig)))
    {
        return str;
    }
    strncpy(buffer, str, p - str);
    buffer[p - str] = '\0';
    sprintf(buffer + (p - str), "%s%s", rep, p + strlen(orig));
    //reemplazar todas las ocurrencias
    if (strstr(buffer, orig) != NULL)
    {
        strcpy(buffer, replace(buffer, orig, rep));
    }
    return buffer;
}

bool contains(const char* str, const char* search)
{
    return strstr(str, search) == NULL ? false : true;
}
/*
char *toUpper(const char* str)
{
    int i = 0;
    int len = strlen(str);
    char *ret = malloc(len + 1);
    for (i = 0; i < len; i++)
    {
        ret[i] = toupper(str[i]);
    }
    ret[len] = 0;
    return ret;
}

char* trim(const char *str)
{
    char *start;
    char *end;
    for (start = (char*) str; *start; start++)
    {
        if (!isspace((unsigned char)start[0]))
            break;
    }
    for (end = start + strlen(start); end > start + 1; end--)
    {
        if (!isspace((unsigned char)end[-1]))
            break;
    }
    char *ret = malloc((end - start) + 1);
    *end = 0;
    if (start > str)
    {
        memcpy(ret, start, (end - start) + 1);
    }
    else
    {
        memcpy(ret, str, strlen(str));
    }
    return ret;
}
*/

list_node* lat_crear_lista_node(void* d)
{
    list_node* ret = (list_node*)lat_asignar_memoria(sizeof(list_node));
    ret->data = d;
    return ret;
}

list_node* lat_crear_lista()
{
    list_node* start = (list_node*)lat_asignar_memoria(sizeof(list_node));
    list_node* end = (list_node*)lat_asignar_memoria(sizeof(list_node));
    start->prev = NULL;
    start->next = end;
    start->data = NULL;
    end->prev = start;
    end->next = NULL;
    end->data = NULL;
    return start;
}

int find_list(list_node* l, void* data)
{
    list_node* c;
    for (c = l; c->next != NULL; c = c->next)
    {
        if (c->data == data)
        {
            return 1;
        }
    }
    return 0;
}

void insert_list(list_node* l, void* data)
{
    list_node* ins = (list_node*)lat_asignar_memoria(sizeof(list_node));
    ins->data = data;
    ins->next = l->next;
    l->next = ins;
    ins->next->prev = ins;
    ins->prev = l;
}

void remove_list(list_node* l, void* data)
{
    list_node* c;
    for (c = l; c->next != NULL; c = c->next)
    {
        if (c->data == data)
        {
            c->prev->next = c->next;
            c->next->prev = c->prev;
        }
    }
}

int length_list(list_node* l)
{
    int a = 0;
    list_node* c;
    for (c = l; c->next != NULL; c = c->next)
    {
        if (c->data != NULL)
        {
            a++;
        }
    }
    return a;
}

hash_map* make_hash_map()
{
    hash_map* ret = (hash_map*)lat_asignar_memoria(sizeof(hash_map));
    int c;
    for (c = 0; c < 256; c++)
    {
        ret->buckets[c] = NULL;
    }
    return ret;
}

int hash(char* key)
{
    int h = 5381;

    unsigned char c;
    for (c = *key; c != '\0'; c = *++key)
        h = h * 33 + c;

    return abs(h % 256);
}

void* get_hash(hash_map* m, char* key)
{
    list_node* cur = m->buckets[hash(key)];
    if (cur == NULL)
        return NULL;
    for (; cur->next != NULL; cur = cur->next)
    {
        if (cur->data != NULL)
        {
            if (strcmp(key, ((hash_val *)cur->data)->key) == 0)
            {
                return ((hash_val *)cur->data)->val;
            }
        }
    }
    return NULL;
}

void set_hash(hash_map *m, char *key, void *val)
{
    hash_val *hv = (hash_val *)lat_asignar_memoria(sizeof(hash_val));
    strncpy(hv->key, key, (strlen(key)+1));
    hv->val = val;
    int hk = hash(key);
    if (m->buckets[hk] == NULL)
    {
        m->buckets[hk] = lat_crear_lista();
    }
    else
    {
        list_node *c;
        for (c = m->buckets[hk]; c->next != NULL; c = c->next)
        {
            if (c->data != NULL)
            {
                if (strcmp(((hash_val *)c->data)->key, key) == 0)
                {
                    free(c->data);
                    c->data = (void *)hv;
                    return;
                }
            }
        }
    }
    insert_list(m->buckets[hk], (void *)hv);
}

hash_map *copy_hash(hash_map *m)
{
    hash_map *ret = make_hash_map();
    int i;
    for (i = 0; i < 256; i++)
    {
        list_node *c = m->buckets[i];
        if (c != NULL)
        {
            for (; c->next != NULL; c = c->next)
            {
                if (c->data != NULL)
                {
                    set_hash(ret, ((hash_val *)c->data)->key, ((hash_val *)c->data)->val);
                }
            }
        }
    }
    return ret;
}

bool legible(const char *archivo)
{
    FILE *f = fopen(archivo, "r");
    if(f == NULL)
    {
        return false;
    }
    fclose(f);
    return true;
}

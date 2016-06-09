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

#include "khash.h"
#include "latino.h"
#include "object.h"
#include "libmem.h"
#include "utils.h"

KHASH_MAP_INIT_INT64(env, lat_objeto);
typedef khash_t(env) lat_env;
lat_env* globals;

struct sym_key
{
    const char* ptr;
    size_t len;
};

static khint_t
sym_hash(struct sym_key key)
{
    const char* s = key.ptr;
    khint_t h;
    size_t len = key.len;
    h = *s++;
    while (len--)
    {
        h = (h << 5) - h + (khint_t)*s++;
    }
    return h;
}

static khint_t
sym_eq(struct sym_key a, struct sym_key b)
{
    if (a.len != b.len)
        return false;
    if (memcmp(a.ptr, b.ptr, a.len) == 0)
        return true;
    return false;
}

KHASH_INIT(sym, struct sym_key, lat_objeto*, 1, sym_hash, sym_eq);
static khash_t(sym) * sym_table;

static lat_objeto* str_new(const char* p, size_t len)
{
    lat_objeto* str = (lat_objeto*)lat_asignar_memoria(sizeof(lat_objeto));
    str->tipo = T_STR;
    str->tamanio_datos = len;
    str->datos.cadena = (char *)p;
    return str;
}

static lat_objeto* str_intern(const char* p, size_t len)
{
    khiter_t k;
    struct sym_key key;
    int ret;
    lat_objeto* str;
    if (!sym_table)
    {
        sym_table = kh_init(sym);
    }
    key.ptr = p;
    key.len = len;
    k = kh_put(sym, sym_table, key, &ret);
    if (ret == 0)
    {
        return kh_value(sym_table, k);
    }
    str = str_new(p, len);
    kh_key(sym_table, k).ptr = str->datos.cadena;
    kh_value(sym_table, k) = str;
    return str;
}

lat_objeto* lat_cadena_hash(const char* p, size_t len)
{
    if (p && (len < MAX_STR_INTERN))
    {
        return str_intern(p, len);
    }
    return str_new(p, len);
}

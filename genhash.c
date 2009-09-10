/*
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "genhash.h"
#include "genhash_int.h"

/* Table of 32 primes by their distance from the nearest power of two */
static int prime_size_table[]={
    3, 7, 13, 23, 47, 97, 193, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157,
    98299, 196613, 393209, 786433, 1572869, 3145721, 6291449, 12582917,
    25165813, 50331653, 100663291, 201326611, 402653189, 805306357,
    1610612741
};

#define TABLE_SIZE ((int)(sizeof(prime_size_table) / sizeof(int)))

int
estimate_table_size(int est)
{
    assert(est > 0);
    int rv=0;
    while(prime_size_table[rv] < est && rv+1 < TABLE_SIZE) {
        rv++;
    }
    return prime_size_table[rv];
}

genhash_t* genhash_init(int est, struct hash_ops ops)
{
    genhash_t* rv=NULL;
    int size=0;
    if (est < 1) {
        return NULL;
    }

    assert(ops.hashfunc != NULL);
    assert(ops.hasheq != NULL);
    assert(ops.dupKey != NULL);
    assert(ops.dupValue != NULL);
    assert(ops.freeKey != NULL);
    assert(ops.freeValue != NULL);

    size=estimate_table_size(est);
    rv=calloc(1, sizeof(genhash_t)
              + (size * sizeof(struct genhash_entry_t *)));
    assert(rv != NULL);
    rv->size=size;
    rv->ops=ops;

    return rv;
}

void
genhash_free(genhash_t* h)
{
    if(h != NULL) {
        genhash_clear(h);
        free(h);
    }
}

void
genhash_store(genhash_t *h, const void* k, const void* v)
{
    size_t n=0;
    struct genhash_entry_t *p;

    assert(h != NULL);

    n=h->ops.hashfunc(k) % h->size;
    assert(n < h->size);

    p=calloc(1, sizeof(struct genhash_entry_t));
    assert(p);

    p->key=h->ops.dupKey(k);
    p->value=h->ops.dupValue(v);

    p->next=h->buckets[n];
    h->buckets[n]=p;
}

static struct genhash_entry_t *
genhash_find_entry(genhash_t *h, const void* k)
{
    size_t n=0;
    struct genhash_entry_t *p;

    assert(h != NULL);
    n=h->ops.hashfunc(k) % h->size;
    assert(n < h->size);

    p=h->buckets[n];
    for(p=h->buckets[n]; p && !h->ops.hasheq(k, p->key); p=p->next);
    return p;
}

void*
genhash_find(genhash_t *h, const void* k)
{
    struct genhash_entry_t *p;
    void *rv=NULL;

    p=genhash_find_entry(h, k);

    if(p) {
        rv=p->value;
    }
    return rv;
}

enum update_type
genhash_update(genhash_t* h, const void* k, const void* v)
{
    struct genhash_entry_t *p;
    enum update_type rv=0;

    p=genhash_find_entry(h, k);

    if(p) {
        h->ops.freeValue(p->value);
        p->value=h->ops.dupValue(v);
        rv=MODIFICATION;
    } else {
        genhash_store(h, k, v);
        rv=NEW;
    }

    return rv;
}

enum update_type
genhash_fun_update(genhash_t* h, const void* k,
                   void *(*upd)(const void *, const void *),
                   void (*fr)(void*),
                   const void *def)
{
    struct genhash_entry_t *p;
    enum update_type rv=0;

    p=genhash_find_entry(h, k);

    if(p) {
        void *newValue=upd(k, p->value);
        h->ops.freeValue(p->value);
        p->value=h->ops.dupValue(newValue);
        fr(newValue);
        rv=MODIFICATION;
    } else {
        void *newValue=upd(k, def);
        genhash_store(h, k, newValue);
        fr(newValue);
        rv=NEW;
    }

    return rv;
}

static void
free_item(genhash_t *h, struct genhash_entry_t *i)
{
    assert(i);
    h->ops.freeKey(i->key);
    h->ops.freeValue(i->value);
    free(i);
}

int
genhash_delete(genhash_t* h, const void* k)
{
    struct genhash_entry_t *deleteme=NULL;
    size_t n=0;
    int rv=0;

    assert(h != NULL);
    n=h->ops.hashfunc(k) % h->size;
    assert(n < h->size);

    if(h->buckets[n] != NULL) {
        /* Special case the first one */
        if(h->ops.hasheq(h->buckets[n]->key, k)) {
            deleteme=h->buckets[n];
            h->buckets[n]=deleteme->next;
        } else {
            struct genhash_entry_t *p=NULL;
            for(p=h->buckets[n]; deleteme==NULL && p->next != NULL; p=p->next) {
                if(h->ops.hasheq(p->next->key, k)) {
                    deleteme=p->next;
                    p->next=deleteme->next;
                }
            }
        }
    }
    if(deleteme != NULL) {
        free_item(h, deleteme);
        rv++;
    }

    return rv;
}

int
genhash_delete_all(genhash_t* h, const void* k)
{
    int rv=0;
    while(genhash_delete(h, k) == 1) {
        rv++;
    }
    return rv;
}

void
genhash_iter(genhash_t* h,
             void (*iterfunc)(const void* key, const void* val, void *arg), void *arg)
{
    size_t i=0;
    struct genhash_entry_t *p=NULL;
    assert(h != NULL);

    for(i=0; i<h->size; i++) {
        for(p=h->buckets[i]; p!=NULL; p=p->next) {
            iterfunc(p->key, p->value, arg);
        }
    }
}

int
genhash_clear(genhash_t *h)
{
    size_t i = 0;
    int rv = 0;
    assert(h != NULL);

    for(i = 0; i < h->size; i++) {
        while(h->buckets[i]) {
            struct genhash_entry_t *p = NULL;
            p = h->buckets[i];
            h->buckets[i] = p->next;
            free_item(h, p);
        }
    }

    return rv;
}

static void
count_entries(const void *key __attribute__((unused)),
              const void *val __attribute__((unused)),
              void *arg)
{
    int *count=(int *)arg;
    (*count)++;
}

int
genhash_size(genhash_t* h) {
    int rv=0;
    assert(h != NULL);
    genhash_iter(h, count_entries, &rv);
    return rv;
}

int
genhash_size_for_key(genhash_t* h, const void* k)
{
    int rv=0;
    assert(h != NULL);
    genhash_iter_key(h, k, count_entries, &rv);
    return rv;
}

void
genhash_iter_key(genhash_t* h, const void* key,
                 void (*iterfunc)(const void* key, const void* val, void *arg), void *arg)
{
    size_t n=0;
    struct genhash_entry_t *p=NULL;

    assert(h != NULL);
    n=h->ops.hashfunc(key) % h->size;
    assert(n < h->size);

    for(p=h->buckets[n]; p!=NULL; p=p->next) {
        if(h->ops.hasheq(key, p->key)) {
            iterfunc(p->key, p->value, arg);
        }
    }
}

int
genhash_string_hash(const void* p)
{
    int rv=5381;
    int i=0;
    char *str=(char *)p;

    for(i=0; str[i] != 0x00; i++) {
        rv = ((rv << 5) + rv) ^ str[i];
    }

    return rv;
}

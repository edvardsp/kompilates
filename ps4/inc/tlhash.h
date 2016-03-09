#ifndef TLHASH_H
#define TLHASH_H

#include <stddef.h>

typedef struct el {
    void *key, *value;
    size_t key_length;
    struct el *next;
} Tlhash_elem;

typedef struct {
    size_t n_buckets, size;
    Tlhash_elem **buckets;
} Tlhash;

typedef Tlhash_elem* pTlhash_elem;
typedef Tlhash* pTlhash;

int    tlhash_init    (pTlhash tab, size_t n_buckets);
int    tlhash_finalize(pTlhash tab);
int    tlhash_insert  (pTlhash tab, void *key, size_t keylen, void *val);
int    tlhash_lookup  (pTlhash tab, void *key, size_t keylen, void **val);
int    tlhash_remove  (pTlhash tab, void *key, size_t key_length);
size_t tlhash_size    (pTlhash tab);
void   tlhash_keys    (pTlhash tab, void **keys);
void   tlhash_values  (pTlhash tab, void **values);

#define TLHASH_SUCCESS 0     /* Success */
#define TLHASH_ENOMEM  1     /* No memory available */
#define TLHASH_ENOENT  2     /* No such table entry */
#define TLHASH_EEXIST  3     /* Table entry already exists */

#endif

#ifndef _NCX_SLAB_H_INCLUDED_
#define _NCX_SLAB_H_INCLUDED_


#include "ncx_core.h"
#include "vxsync.h"
#include "ncx_log.h"

typedef struct ncx_slab_page_s  ncx_slab_page_t;

struct ncx_slab_page_s {
    vxuintptr         slab;
    ncx_slab_page_t  *next;
	vxuintptr         prev;
};


typedef struct ncx_slab_pool_t{
    size_t            min_size;
    size_t            min_shift;

    ncx_slab_page_t  *pages;
    ncx_slab_page_t   free;

    u_char           *start;
    u_char           *end;

	VXTHREAD_MUTEX	 mutex;
    void             *addr;
} ncx_slab_pool_t;

#define PAGE_MERGE

void ncx_slab_init(ncx_slab_pool_t *pool);
void *ncx_slab_alloc(ncx_slab_pool_t *pool, size_t size);
void *ncx_slab_alloc_locked(ncx_slab_pool_t *pool, size_t size);
void ncx_slab_free(ncx_slab_pool_t *pool, void *p);
void ncx_slab_free_locked(ncx_slab_pool_t *pool, void *p);

void ncx_slab_dummy_init(ncx_slab_pool_t *pool);
void ncx_slab_stat(ncx_slab_pool_t *pool, ncx_slab_stat_t *stat);

#endif /* _NCX_SLAB_H_INCLUDED_ */

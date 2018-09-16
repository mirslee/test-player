#include "../stdafx.h"
#include "ncx_slab.h"
#ifndef _WIN32
#include <unistd.h>
#endif
#include "MxTypes.h"
#include "MxSynchronize.h"

#define NCX_SLAB_PAGE_MASK   3
#define NCX_SLAB_PAGE        0
#define NCX_SLAB_BIG         1
#define NCX_SLAB_EXACT       2
#define NCX_SLAB_SMALL       3

#if (NCX_PTR_SIZE == 4)  //32λϵͳ

#define NCX_SLAB_PAGE_FREE   0
#define NCX_SLAB_PAGE_BUSY   0xffffffff
#define NCX_SLAB_PAGE_START  0x80000000

#define NCX_SLAB_SHIFT_MASK  0x0000000f
#define NCX_SLAB_MAP_MASK    0xffff0000
#define NCX_SLAB_MAP_SHIFT   16

#define NCX_SLAB_BUSY        0xffffffff

#else /* (NCX_PTR_SIZE == 8) */

#define NCX_SLAB_PAGE_FREE   0
#define NCX_SLAB_PAGE_BUSY   0xffffffffffffffff
#define NCX_SLAB_PAGE_START  0x8000000000000000

#define NCX_SLAB_SHIFT_MASK  0x000000000000000f
#define NCX_SLAB_MAP_MASK    0xffffffff00000000
#define NCX_SLAB_MAP_SHIFT   32

#define NCX_SLAB_BUSY        0xffffffffffffffff

#endif


#if (NCX_DEBUG_MALLOC)

#define ncx_slab_junk(p, size)     ncx_memset(p, 0xA5, size)

#else

#define ncx_slab_junk(p, size)

#endif


static ncx_slab_page_t *ncx_slab_alloc_pages(ncx_slab_pool_t *pool,
    ncx_uint_t pages);
static void ncx_slab_free_pages(ncx_slab_pool_t *pool, ncx_slab_page_t *page,
    ncx_uint_t pages);
static bool ncx_slab_empty(ncx_slab_pool_t *pool, ncx_slab_page_t *page);

static ncx_uint_t  ncx_slab_max_size;
static ncx_uint_t  ncx_slab_exact_size;
static ncx_uint_t  ncx_slab_exact_shift;
static ncx_uint_t  ncx_pagesize;
static ncx_uint_t  ncx_pagesize_shift;
static ncx_uint_t  ncx_real_pages;

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
int getpagesize() {
	static int pagesize = 0;
	if (pagesize == 0) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		pagesize = system_info.dwPageSize > system_info.dwAllocationGranularity ? system_info.dwPageSize : system_info.dwAllocationGranularity;
	}
	return pagesize;
}
#endif

#pragma warning(disable:4334)
void
ncx_slab_init(ncx_slab_pool_t *pool)
{
    u_char           *p;
    size_t            size;
    ncx_uint_t        i, n, pages;
    ncx_slab_page_t  *slots;

	/*
	* ��ʼ��pagesize Ϊϵͳ�ڴ�ҳ��С��һ��Ϊ4k
	* pagesize = pow(2, pagesize_shift)����xxx_shift����ָ��ָ��
	* һ��page���и�ɴ�С��ȵ��ڴ��(�����ע���ݳ�Ϊobj)��
	* ��ͬ��page�����и��obj��С���ܲ��ȣ���obj�Ĵ�С����2��N�η������.�� 8 16 32 ...
	* */
	ncx_pagesize = getpagesize();
	for (n = ncx_pagesize, ncx_pagesize_shift = 0; 
			n >>= 1; ncx_pagesize_shift++) { /* void */ }

	/*
	* slab_max_size ,��slab���obj��С��Ĭ��Ϊpagesize��һ��
	* slab_exact_size, һ���ٽ�ֵ�������ݲ���ϸ���ۣ��������ע��.
	*/
    if (ncx_slab_max_size == 0) {
        ncx_slab_max_size = ncx_pagesize / 2;
        ncx_slab_exact_size = ncx_pagesize / (8 * sizeof(mxuvoidptr));
        for (n = ncx_slab_exact_size; n >>= 1; ncx_slab_exact_shift++) {
            /* void */
        }
    }

	mxMutexInit(&pool->mutex);
	/*
	* ��С��obj��С��nginxĬ��ʹ��8�ֽ�
	* ��min_shift Ϊ��Сobj��С����ָ�� <=> 8 = pow(2, 3)
	*/
    pool->min_size = 1 << pool->min_shift;

    p = (u_char *) pool + sizeof(ncx_slab_pool_t);
    slots = (ncx_slab_page_t *) p;

    n = ncx_pagesize_shift - pool->min_shift;

	/*
	* slots��page����ڵ��ͷ�ڵ㣬һ��slots����(��slots[n])���page��obj��С���
	* slots[n].slab ��һ��vxuintptr���ͣ���32λϵͳ����unsigned�ȼۣ�����32bit����
	* slab�ֶ��ڲ�ͬ��ʹ�ó����´���ͬ���壬��Ҳ��nginx slab����ĵط�֮һ.
	* ��ϸ���ͼ�����ע��.
	*/
    for (i = 0; i < n; i++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

    p += n * sizeof(ncx_slab_page_t);

    size = pool->end - p;

	/*
	* ģ�������ݣ���ÿ��alloc���ڴ涼��������
	*/
    ncx_slab_junk(p, size);

	/*
	* ÿ��page�ָ����ȴ�С��obj, һ��page��Ӧһ������ڵ�
	* pages�����ڴ�صĴ�С�ɷ�����ٸ�page
	* ������Ҫ�����ڴ��������⣬������init������β�����¼���pages�Ĵ�С
	* ���ԣ��ڴ���������ڴ��˷�(��󲻳���pagesize*2)����Ҳ������ܶྪϲ...
	*/
    pages = (ncx_uint_t) (size / (ncx_pagesize + sizeof(ncx_slab_page_t)));

    ncx_memzero(p, pages * sizeof(ncx_slab_page_t));

    pool->pages = (ncx_slab_page_t *) p;

	/*
	* 1.free�����������п��е�page�Ĺ���ڵ�
	* 2.slots���������������õģ������п���obj��page�Ĺ���ڵ�
	* 3.��û�п���obj��page�Ĺ���ڵ㣬���ڡ����ա�״̬���Ȳ���free��Ҳ����slots��.
	*
	* �������3��page���ڴ�ش���"���ɼ�"״̬����Ӧ�ò�����ڴ��ʱ������ȥ��ѯ����page
	* ֻ����free���ͷ��ڴ��ʱ�򣬻ᱻ���·ŵ���Ӧ��slots������Ϊ��ʱ�����п��ýڵ���
	*
	* ������free������slots�������ӵĶ���page��Ӧ�Ĺ���ڵ㣬page�Ǵ��ʵ�����ݵ�,������û�а���ϵ.
	*/
    pool->free.prev = 0;
    pool->free.next = (ncx_slab_page_t *) p;

	/*
	* pool->pagesָ���һ��page��Ӧ�Ĺ���ڵ�
	* ��ʱslab�ֶα�ʾ���������Ŀ��õ�page��������ʼ��ʱ��Ϊ�����ҿ��ã���Ϊpages
	* ���Զ���δʹ�õ�page�����Ӧ�Ĺ���ڵ��slab��ʾ�Ը�page��ʼ���������õ�page����
	* ��ʹ�õĹ����У������ķ��䡢���գ����õ�page֮����ͨ���������ӣ��������ڴ��ﲢ��һ����������
	*/
    pool->pages->slab = pages;
    pool->pages->next = &pool->free;
    pool->pages->prev = (mxuvoidptr) &pool->free;

	/*
	* startָ���һ��page���׵�ַ
	* ֵ��ע������������׵�ַ������pagesize���룬�����ܶ�������������ڴ����
	*/
    pool->start = (u_char *)
                  ncx_align_ptr((mxuvoidptr) p + pages * sizeof(ncx_slab_page_t),
                                 ncx_pagesize);

	/*
	* ���¼�����һ���ڴ����󣬿��õ�page����
	* ��Ҫ���ǵ���page �� page����ڵ���һһ��Ӧ��
	* �����ܶ�����Ǹ���page����ڵ���±�������pageҳ��ʵ�ʵ�ַ.
	*/
	ncx_real_pages = (pool->end - pool->start) / ncx_pagesize;
	pool->pages->slab = ncx_real_pages;
}


void *
ncx_slab_alloc(ncx_slab_pool_t *pool, size_t size)
{
	/*
	 * �ṩ��һ�������ӿڣ������ncx_lock.h
	 *
	 * ����ڴ���ǻ��ڹ����ڴ���䣬��ͬʱ��������̹���
	 * ����ʵ��һ�����̼���������(�ɲο�nginx��ngx_shmtx.c)
	 * 
	 * ����Ƕ��̹߳������ʹ���̼߳���������
	 * �� pthread_spin_lock
	 *
	 * �����˽���ڴ棬�����ǵ����̵��߳�ģ��
	 * ���ncx_shmtx_lock/unlock �ɶ���Ϊ��
	 */
	CMxMutexLocker lock(&pool->mutex);
    return ncx_slab_alloc_locked(pool, size);
}


void *
ncx_slab_alloc_locked(ncx_slab_pool_t *pool, size_t size)
{
    size_t            s;
    mxuvoidptr         p, n, m, mask, *bitmap;
    ncx_uint_t        i, slot, shift, map;
    ncx_slab_page_t  *page, *prev, *slots;

	/*
	 * �����Ҫ������ڴ泬������obj��С������pagesizeΪ��λ������ҳ����
	 */
    if (size >= ncx_slab_max_size) {

		debug("slab alloc: %zu", size);

        page = ncx_slab_alloc_pages(pool, (size >> ncx_pagesize_shift)
                                          + ((size % ncx_pagesize) ? 1 : 0));

		/*
		 * ����ÿ��page�������ڵ���һһ��Ӧ�ģ����Ը��ݹ���ڵ��ƫ�ƣ������׿ɼ����page���׵�ַ
		 * 1 << ncx_pagesize_shift ��һ��page�Ĵ�С
		 */
        if (page) {
            p = (page - pool->pages) << ncx_pagesize_shift;
            p += (mxuvoidptr) pool->start;

        } else {
            p = 0;
        }

        goto done;
    }

	/*
	 * ����size���������Ӧ�ĸ�slot��
	 * ������СobjΪ8�ֽڣ����objΪ2048�ֽڣ���slab��9����ģ���ֱ�Ϊ
	 * 8 16 32 64 128 256 512 1024 2048
	 */
    if (size > pool->min_size) {
        shift = 1;
        for (s = size - 1; s >>= 1; shift++) { /* void */ }
        slot = shift - pool->min_shift;

    } else {
        size = pool->min_size;
        shift = pool->min_shift;
        slot = 0;
    }

	/*
	 * ��ȡ��Ӧ��page����ڵ�����
	 */
    slots = (ncx_slab_page_t *) ((u_char *) pool + sizeof(ncx_slab_pool_t));
    page = slots[slot].next;

	/*
	 * slab���ڿ���obj
	 */
    if (page->next != page) {

		/*
		 * slab��ģ��Ϊ����:
		 *  1. < ncx_slab_exact_shift
		 *  2. = ncx_slab_exact_shift
		 *  3. > ncx_slab_exact_shift 
		 *
		 * ΪʲôҪ�����������������ncx_slab_page_t.slab�ֶε�ʹ��ֱ�����;
		 * ��32λ����ϵͳ��slab��С��4�ֽڣ���32bit
		 * ��Ӧ�������������slab�ֶε�ʹ�÷ֱ���
		 *  1.��ʾ�����obj��С����¼����size_shift
		 *  2.pagesize/slab_exact_size �պ�Ϊ32������slab���Ե���bitmapʹ�ã���ʾpage����Щobj����
		 *  3.��(16)λ��bitmap����(16)λ�Ǽ�¼���С; 
		 */
        if (shift < ncx_slab_exact_shift) {

            do {
				/*
				 * < slab_exact_shift�������slabֻ������¼obj�Ĵ�С
				 * ��page��bitmap����Ҫռ��obj�����
				 */
                p = (page - pool->pages) << ncx_pagesize_shift;
                bitmap = (mxuvoidptr *) (pool->start + p);

				/*
				 * (1 << (ncx_pagesize_shift - shift)) ���һ��page��ŵ�obj��Ŀ
				 *  / (sizeof(vxuintptr) * 8) ������Ҫռ�ö���vxuintptr ���洢bitmap
				 *
				 *  ��map ��ʾռ�ö��ٸ�vxuintptr��32bitϵͳռ4�ֽڣ�64bitռ8�ֽڣ�
				 */
                map = (1 << (ncx_pagesize_shift - shift))
                          / (sizeof(mxuvoidptr) * 8);

                for (n = 0; n < map; n++) {

					/*
					 * ����п���obj
					 */
                    if (bitmap[n] != NCX_SLAB_BUSY) {

						/*
						 * �ҳ�����Ŀ���obj
						 * bitΪ0Ϊ���нڵ�
						 */
                        for (m = 1, i = 0; m; m <<= 1, i++) {
                            if ((bitmap[n] & m)) {
                                continue;
                            }

							/*
							 * ����bitmap
							 */
                            bitmap[n] |= m;

							/*
							 *  ����ó�iΪ�ÿ���obj��page��ĵ�ַƫ��
							 *  �ɲ��Ϊ (1<<shift) * ((n*sizeof(vxuintptr)*8) + i) ���
							 *  ((n*sizeof(vxuintptr)*8) + i) ��ʾ��n��obj
							 *  (1<<shift) Ϊһ��obj�Ĵ�С
							 */
                            i = ((n * sizeof(mxuvoidptr) * 8) << shift)
                                + (i << shift);

							/*
							 * ���������߼��Ǳ�������bitmap
							 * �����pageû�п���obj����Ѹ�page�Ĺ���ڵ��������ɾ��������page���ڡ����ա�״̬
							 * ���ԣ���һ��ȷ��,slots���������ӵ����п���obj��page�Ĺ���ڵ�
							 */
                            if (bitmap[n] == NCX_SLAB_BUSY) {
                                for (n = n + 1; n < map; n++) {
                                     if (bitmap[n] != NCX_SLAB_BUSY) {
                                         p = (mxuvoidptr) bitmap + i;

                                         goto done;
                                     }
                                }

								/*
								 * (page->prev & ~NCX_SLAB_PAGE_MASK) ��ȡԭʼ��prev��page�ĵ�ַ
								 * nginx���뾭�������ڴ����,��ô����������ܵ���������ͬʱ�����С�����ڴ��˷�
								 * ����nginx�ܶ�ʱ����ĳС������Ϣ�������ڵ�ַ�ͨ���򵥵ġ��򡱣����롱���������úͻ�ԭ.
								 *
								 * prev���ص���Ϣ��page��Ӧ�Ĺ�ģ����(small.exact,big.page).�������ϸ���ۡ�
								 */
                                prev = (ncx_slab_page_t *)
                                            (page->prev & ~NCX_SLAB_PAGE_MASK);
                                prev->next = page->next;
                                page->next->prev = page->prev;

								/*
								 * NCX_SLAB_SMALL ��ʾ < ncx_slab_exact_shift ��slab����
								 */
                                page->next = NULL;
                                page->prev = NCX_SLAB_SMALL;
                            }

							/*
							 * ��������ע�ͣ�i��ʾobj�ڸ�page(page)���ƫ��
							 */
                            p = (mxuvoidptr) bitmap + i;

                            goto done;
                        }
                    }
                }

                page = page->next;

            } while (page);

			/*
			 * slab_exact ���ͣ�����pagesizeΪ4096,
			 * ��slab_exact_size Ϊ128
			 */
        } else if (shift == ncx_slab_exact_shift) {

            do {
                if (page->slab != NCX_SLAB_BUSY) {

					/*
					 * slab�ֶ�����bitmap
					 * forѭ����Ϊ���ҵ����е�obj
					 */
                    for (m = 1, i = 0; m; m <<= 1, i++) {
                        if ((page->slab & m)) {
                            continue;
                        }

                        page->slab |= m;
  
                        if (page->slab == NCX_SLAB_BUSY) {
                            prev = (ncx_slab_page_t *)
                                            (page->prev & ~NCX_SLAB_PAGE_MASK);
                            prev->next = page->next;
                            page->next->prev = page->prev;

                            page->next = NULL;
                            page->prev = NCX_SLAB_EXACT;
                        }

						/*
						 * (page - pool->pages) << ncx_pagesize_shift �����page���׵�ַ
						 * i << shift ���obj��page���ƫ��
						 * += pool->start ���obj���׵�ַ
						 */
                        p = (page - pool->pages) << ncx_pagesize_shift;
                        p += i << shift;
                        p += (mxuvoidptr) pool->start;

                        goto done;
                    }
                }

                page = page->next;

            } while (page);

        } else {
			/* 
			 *  > ncx_slab_exact_shift
			 *  (page->slab & NCX_SLAB_SHIFT_MASK) => ȡpage��Ӧ��obj��size_shift
			 *  1 << n ���page��洢��obj��
			 *  ((vxuintptr) 1 << n) - 1 => ���bitmap ����
			 *  n << NCX_SLAB_MAP_SHIFT ��Ϊ�Ǹ�λ����bitmap���ݣ�������Ҫ��������λ��
			 */
            n = ncx_pagesize_shift - (page->slab & NCX_SLAB_SHIFT_MASK);
            n = 1 << n;
            n = ((mxuvoidptr) 1 << n) - 1;
            mask = n << NCX_SLAB_MAP_SHIFT;
 
			/*
			 * �������Ĳ�����֮ǰ����
			 */
            do {
                if ((page->slab & NCX_SLAB_MAP_MASK) != mask) {

                    for (m = (mxuvoidptr) 1 << NCX_SLAB_MAP_SHIFT, i = 0;
                         m & mask;
                         m <<= 1, i++)
                    {
                        if ((page->slab & m)) {
                            continue;
                        }

                        page->slab |= m;

                        if ((page->slab & NCX_SLAB_MAP_MASK) == mask) {
                            prev = (ncx_slab_page_t *)
                                            (page->prev & ~NCX_SLAB_PAGE_MASK);
                            prev->next = page->next;
                            page->next->prev = page->prev;

                            page->next = NULL;
                            page->prev = NCX_SLAB_BIG;
                        }

                        p = (page - pool->pages) << ncx_pagesize_shift;
                        p += i << shift;
                        p += (mxuvoidptr) pool->start;

                        goto done;
                    }
                }

                page = page->next;

            } while (page);
        }
    }

	/*
	 * ���slots����Ϊ�գ���û�п��õ�pagee
	 * �����·���һ��page,���������ڵ�ŵ�slots����
	 */
    page = ncx_slab_alloc_pages(pool, 1);

    if (page) {
        if (shift < ncx_slab_exact_shift) {
            p = (page - pool->pages) << ncx_pagesize_shift;
            bitmap = (mxuvoidptr *) (pool->start + p);

			/*
			 * nΪ��Ҫ���ٸ�obj�������bitmap
			 */
            s = 1 << shift;
            n = (1 << (ncx_pagesize_shift - shift)) / 8 / s;

            if (n == 0) {
                n = 1;
            }

			/*
			 * ����������˵��n����ռ�ö��ٸ�obj�����bitmap
			 * ���ԣ�bitmap[0]��ʼ����Ҫ��ռ�õ�obj��Ӧ��bit��Ϊ1
			 */
            bitmap[0] = (2 << n) - 1;

			/*
			 * mapΪbitmap�����ǵ�vxuintptr��
			 */
            map = (1 << (ncx_pagesize_shift - shift)) / (sizeof(uintptr_t) * 8);

            for (i = 1; i < map; i++) {
                bitmap[i] = 0;
            }

			/*
			 * ���·����page��Ӧ�Ĺ���ڵ�ŵ�slots����
			 * (vxuintptr) &slots[slot] | NCX_SLAB_SMALL����prev�ֶ��ﱣ����slab�Ĺ�ģ(small,exact,big,page����)
			 * �������ĺô���Ҫ�Ǽ���free���߼�;��free��������ϸ����
			 */
            page->slab = shift;
            page->next = &slots[slot];
            page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_SMALL;

            slots[slot].next = page;

            p = ((page - pool->pages) << ncx_pagesize_shift) + s * n;
            p += (mxuvoidptr) pool->start;

            goto done;

        } else if (shift == ncx_slab_exact_shift) {

            page->slab = 1;
            page->next = &slots[slot];
            page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_EXACT;

            slots[slot].next = page;

            p = (page - pool->pages) << ncx_pagesize_shift;
            p += (mxuvoidptr) pool->start;

            goto done;

        } else { /* shift > ncx_slab_exact_shift */

            page->slab = ((mxuvoidptr) 1 << NCX_SLAB_MAP_SHIFT) | shift;
            page->next = &slots[slot];
            page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_BIG;

            slots[slot].next = page;

            p = (page - pool->pages) << ncx_pagesize_shift;
            p += (mxuvoidptr) pool->start;

            goto done;
        }
    }

    p = 0;

done:

    debug("slab alloc: %p", (void *)p);

    return (void *) p;
}


void ncx_slab_free(ncx_slab_pool_t *pool, void *p)
{
	CMxMutexLocker lock(&pool->mutex);
    ncx_slab_free_locked(pool, p);
}


void
ncx_slab_free_locked(ncx_slab_pool_t *pool, void *p)
{
    size_t            size;
    mxuvoidptr         slab, m, *bitmap;
    ncx_uint_t        n, type, slot, shift, map;
    ncx_slab_page_t  *slots, *page;

    debug("slab free: %p", p);

    if ((u_char *) p < pool->start || (u_char *) p > pool->end) {
        error("ncx_slab_free(): outside of pool");
        goto fail;
    }

	/*
	 * ���p���ڵ��ǵ�n��page
	 * type Ϊpage��obj�Ĺ�ģ:
	 *  1. SMALL �� < slab_exact_size
	 *  2. EXACT �� = slab_exact_size
	 *  3. BIG   �� > slab_exact_size && < max_slab_size
	 *  4. PAGE  �� > max_slab_size
	 *
	 *  ��ͬ�Ĺ�ģ��free�߼��᲻һ�����������ע��
	 */
    n = ((u_char *) p - pool->start) >> ncx_pagesize_shift;
    page = &pool->pages[n];
    slab = page->slab;
    type = page->prev & NCX_SLAB_PAGE_MASK;

    switch (type) {

    case NCX_SLAB_SMALL:

		/*
		 * slab�������obj�Ĵ�С��shift
		 */
        shift = slab & NCX_SLAB_SHIFT_MASK;
        size = 1 << shift;

		/*
		 * �������ڴ���룬�����Ϸ�����У��
		 */
        if ((mxuvoidptr) p & (size - 1)) {
            goto wrong_chunk;
        }

		/*
		 * 1.���p��Ӧpage��ĵ�n��obj
		 * 2.���obj����Ӧĳ��(�����Ŀ��ǲ���3���)bitmap�ĵ�m��bit
		 * 3.���obj���ڵ����Ŀ�bitmap(һ��vxuintptrΪһ��bitmap)
		 * 4.���bitmap���׵�ַ(����һ��bitmap�ĵ�ַ)
		 */
        n = ((mxuvoidptr) p & (ncx_pagesize - 1)) >> shift;
        m = (mxuvoidptr) 1 << (n & (sizeof(mxuvoidptr) * 8 - 1));
        n /= (sizeof(mxuvoidptr) * 8);
        bitmap = (mxuvoidptr *) ((mxuvoidptr) p & ~(ncx_pagesize - 1));

		/*
		 * ����Ƿ��Ӧ��bit����λ�ˣ��������ֱ�ӷ������ͷ�
		 * �����ظ��ͷŴ���������
		 */
        if (bitmap[n] & m) {

			/*
			 * �����free�Ժ�ʹ��page��busy=>���ã�
			 * ��Ѹ�page���·ŵ�slots�������
			 */
            if (page->next == NULL) {
                slots = (ncx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ncx_slab_pool_t));
                slot = shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_SMALL;
                page->next->prev = (mxuvoidptr) page | NCX_SLAB_SMALL;
            }

            bitmap[n] &= ~m;

			/*
			 * ���bitmapռ��n��obj
			 */
            n = (1 << (ncx_pagesize_shift - shift)) / 8 / (1 << shift);

            if (n == 0) {
                n = 1;
            }

			/*
			 * ����page�Ƿ���ȫ���У����Ƿ�������obj
			 * ���û�У����page���·Ż�free����
			 */
            if (bitmap[0] & ~(((mxuvoidptr) 1 << n) - 1)) {
                goto done;
            }

            map = (1 << (ncx_pagesize_shift - shift)) / (sizeof(mxuvoidptr) * 8);

            for (n = 1; n < map; n++) {
                if (bitmap[n]) {
                    goto done;
                }
            }

            ncx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NCX_SLAB_EXACT:

		/*
		 * p��Ӧbitmap�ĵ�m��bit
		 */
        m = (mxuvoidptr) 1 <<
                (((mxuvoidptr) p & (ncx_pagesize - 1)) >> ncx_slab_exact_shift);
        size = ncx_slab_exact_size;

        if ((mxuvoidptr) p & (size - 1)) {
            goto wrong_chunk;
        }

        if (slab & m) {
            if (slab == NCX_SLAB_BUSY) {
                slots = (ncx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ncx_slab_pool_t));
                slot = ncx_slab_exact_shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_EXACT;
                page->next->prev = (mxuvoidptr) page | NCX_SLAB_EXACT;
            }

            page->slab &= ~m;

            if (page->slab) {
                goto done;
            }

            ncx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NCX_SLAB_BIG:

        shift = slab & NCX_SLAB_SHIFT_MASK;
        size = 1 << shift;

        if ((mxuvoidptr) p & (size - 1)) {
            goto wrong_chunk;
        }

		/*
		 * (((vxuintptr) p & (ncx_pagesize - 1)) >> shift) �����Ӧbitmap�ĸ�bit
		 */
        m = (mxuvoidptr) 1 << ((((mxuvoidptr) p & (ncx_pagesize - 1)) >> shift)
                              + NCX_SLAB_MAP_SHIFT);

        if (slab & m) {

            if (page->next == NULL) {
                slots = (ncx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ncx_slab_pool_t));
                slot = shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (mxuvoidptr) &slots[slot] | NCX_SLAB_BIG;
                page->next->prev = (mxuvoidptr) page | NCX_SLAB_BIG;
            }

            page->slab &= ~m;

            if (page->slab & NCX_SLAB_MAP_MASK) {
                goto done;
            }

            ncx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NCX_SLAB_PAGE:

        if ((mxuvoidptr) p & (ncx_pagesize - 1)) {
            goto wrong_chunk;
        }

		if (slab == NCX_SLAB_PAGE_FREE) {
			alert("ncx_slab_free(): page is already free");
			goto fail;
        }

		if (slab == NCX_SLAB_PAGE_BUSY) {
			alert("ncx_slab_free(): pointer to wrong page");
			goto fail;
        }

        n = ((u_char *) p - pool->start) >> ncx_pagesize_shift;
        size = slab & ~NCX_SLAB_PAGE_START;

        ncx_slab_free_pages(pool, &pool->pages[n], size);

        ncx_slab_junk(p, size << ncx_pagesize_shift);

        return;
    }

    /* not reached */

    return;

done:

    ncx_slab_junk(p, size);

    return;

wrong_chunk:

	error("ncx_slab_free(): pointer to wrong chunk");

    goto fail;

chunk_already_free:

	error("ncx_slab_free(): chunk is already free");

fail:

    return;
}


static ncx_slab_page_t *
ncx_slab_alloc_pages(ncx_slab_pool_t *pool, ncx_uint_t pages)
{
    ncx_slab_page_t  *page, *p;
	
    for (page = pool->free.next; page != &pool->free; page = page->next) {
		/*
		 * �����ᵽ�������ڿ��е�page�����Ӧ�Ĺ���ڵ��slab�ֶα�ʾ�Ը�page��ʼ���������õ�page��
		 * ֵ��ע����ǣ�������õĿ����ڴ�(page)�ܺͳ���size���������ڲ��������ģ�Ҳ�ᵼ�·���ʧ��
		 * nginx�����ϵ����������Ͻ������п��ܰ�ʵ���������ڴ浱������������
		 * ���������������⣬�����ϱ��˲��� http://www.dcshi.com/?p=360 m��ͼ�����
		 */
        if (page->slab >= pages) {

			/*
			 * ���������page����pagesҪ�󣬽��зָ��ʣ���page�Ż�free������
			 */
            if (page->slab > pages) {
				/*
				 * ������page��Ҫ ��ȥ pages
				 */
                page[pages].slab = page->slab - pages;
                page[pages].next = page->next;
                page[pages].prev = page->prev;

                p = (ncx_slab_page_t *) page->prev;
                p->next = &page[pages];
                page->next->prev = (mxuvoidptr) &page[pages];

            } else {
                p = (ncx_slab_page_t *) page->prev;
                p->next = page->next;
                page->next->prev = page->prev;
            }

			/*
			 * slabʹ��:
			 * ������page����������slab ��¼������Ϣ
			 *  1.��ʶ����page����,�� NCX_SLAB_PAGE_START
			 *  2.��ʶ���η����page����, ��pages
			 *
			 * next,prev ����ָ�붼��������״̬���ᵼ�³���"Ұָ��"������ô��
			 * �϶��ǲ���ģ�ֻ��Ҫfree��ʱ�����Ż�free������.
			 */
            page->slab = pages | NCX_SLAB_PAGE_START;
            page->next = NULL;
            page->prev = NCX_SLAB_PAGE;

            if (--pages == 0) {
                return page;
            }

			/*
			 * һ�η��䳬��һ��page������Ҫ�ѵ�һ������page��Ӧ�Ĺ���ṹҲ���и���
			 */
            for (p = page + 1; pages; pages--) {
                p->slab = NCX_SLAB_PAGE_BUSY;
                p->next = NULL;
                p->prev = NCX_SLAB_PAGE;
                p++;
            }

            return page;
        }
	}

    error("ncx_slab_alloc() failed: no memory");

    return NULL;
}

static void
ncx_slab_free_pages(ncx_slab_pool_t *pool, ncx_slab_page_t *page,
    ncx_uint_t pages)
{
    ncx_slab_page_t  *prev, *next;

	if (pages > 1) {
		ncx_memzero(&page[1], (pages - 1)* sizeof(ncx_slab_page_t));
	}  

    if (page->next) {
        prev = (ncx_slab_page_t *) (page->prev & ~NCX_SLAB_PAGE_MASK);
        prev->next = page->next;
        page->next->prev = page->prev;
    }

	page->slab = pages;
	page->prev = (mxuvoidptr) &pool->free;
	page->next = pool->free.next;
	page->next->prev = (mxuvoidptr) page;

	pool->free.next = page;

#define PAGE_MERGE
#ifdef PAGE_MERGE
	if (pool->pages != page) {
		prev = page - 1;
		if (ncx_slab_empty(pool, prev)) {
			for (; prev >= pool->pages; prev--) {
				if (prev->slab != 0) 
				{
					pool->free.next = page->next;
					page->next->prev = (mxuvoidptr) &pool->free;

					prev->slab += pages;
					ncx_memzero(page, sizeof(ncx_slab_page_t));

					page = prev;

					break;
				}
			}
		}
	}

	if ((page - pool->pages + page->slab) < ncx_real_pages) {
		next = page + page->slab;
		if (ncx_slab_empty(pool, next)) 
		{
			prev = (ncx_slab_page_t *) (next->prev);
			prev->next = next->next;
			next->next->prev = next->prev;

			page->slab += next->slab;
			ncx_memzero(next, sizeof(ncx_slab_page_t));
		}	
	}

#endif
}

void
ncx_slab_dummy_init(ncx_slab_pool_t *pool)
{
    ncx_uint_t n;

	/*
	 * �ڴ�ػ��ڹ����ڴ�ʵ�ֵĳ���
	 * �ⲿ����attchͬһ���ڴ治��Ҫ���³�ʼ��ncx_slab_pool_t
	*/
	ncx_pagesize = getpagesize();
	for (n = ncx_pagesize, ncx_pagesize_shift = 0; 
			n >>= 1; ncx_pagesize_shift++) { /* void */ }

    if (ncx_slab_max_size == 0) {
        ncx_slab_max_size = ncx_pagesize / 2;
        ncx_slab_exact_size = ncx_pagesize / (8 * sizeof(mxuvoidptr));
        for (n = ncx_slab_exact_size; n >>= 1; ncx_slab_exact_shift++) {
            /* void */
        }
    }
}

void
ncx_slab_stat(ncx_slab_pool_t *pool, ncx_slab_stat_t *stat)
{
	mxuvoidptr 			m, n, mask, slab;
	mxuvoidptr 			*bitmap;
	mxuvoidptr 			i, j, map, type, obj_size;
	ncx_slab_page_t 	*page;

	ncx_memzero(stat, sizeof(ncx_slab_stat_t));

	page = pool->pages;
 	stat->pages = (pool->end - pool->start) / ncx_pagesize;;

	for (i = 0; i < stat->pages; i++)
	{
		slab = page->slab;
		type = page->prev & NCX_SLAB_PAGE_MASK;

		switch (type) {

			case NCX_SLAB_SMALL:
	
				n = (page - pool->pages) << ncx_pagesize_shift;
                bitmap = (mxuvoidptr *) (pool->start + n);

				obj_size = 1 << slab;
                map = (1 << (ncx_pagesize_shift - slab))
                          / (sizeof(mxuvoidptr) * 8);

				for (j = 0; j < map; j++) {
					for (m = 1 ; m; m <<= 1) {
						if ((bitmap[j] & m)) {
							stat->used_size += obj_size;
							stat->b_small   += obj_size;
						}

					}		
				}
	
				stat->p_small++;

				break;

			case NCX_SLAB_EXACT:

				if (slab == NCX_SLAB_BUSY) {
					stat->used_size += sizeof(mxuvoidptr) * 8 * ncx_slab_exact_size;
					stat->b_exact   += sizeof(mxuvoidptr) * 8 * ncx_slab_exact_size;
				}
				else {
					for (m = 1; m; m <<= 1) {
						if (slab & m) {
							stat->used_size += ncx_slab_exact_size;
							stat->b_exact    += ncx_slab_exact_size;
						}
					}
				}

				stat->p_exact++;

				break;

			case NCX_SLAB_BIG:

				j = ncx_pagesize_shift - (slab & NCX_SLAB_SHIFT_MASK);
				j = 1 << j;
				j = ((mxuvoidptr) 1 << j) - 1;
				mask = j << NCX_SLAB_MAP_SHIFT;
				obj_size = 1 << (slab & NCX_SLAB_SHIFT_MASK);

				for (m = (mxuvoidptr) 1 << NCX_SLAB_MAP_SHIFT; m & mask; m <<= 1)
				{
					if ((page->slab & m)) {
						stat->used_size += obj_size;
						stat->b_big     += obj_size;
					}
				}

				stat->p_big++;

				break;

			case NCX_SLAB_PAGE:

				if (page->prev == NCX_SLAB_PAGE) {		
					slab 			=  slab & ~NCX_SLAB_PAGE_START;
					stat->used_size += slab * ncx_pagesize;
					stat->b_page    += slab * ncx_pagesize;
					stat->p_page    += slab;

					i += (slab - 1);

					break;
				}

			default:

				if (slab  > stat->max_free_pages) {
					stat->max_free_pages = page->slab;
				}

				stat->free_page += slab;

				i += (slab - 1);

				break;
		}

		page = pool->pages + i + 1;
	}

	stat->pool_size = pool->end - pool->start;
	stat->used_pct = stat->used_size * 100 / stat->pool_size;

	info("pool_size : %zu bytes",	stat->pool_size);
	info("used_size : %zu bytes",	stat->used_size);
	info("used_pct  : %zu%%\n",		stat->used_pct);

	info("total page count : %zu",	stat->pages);
	info("free page count  : %zu\n",	stat->free_page);
		
	info("small slab use page : %zu,\tbytes : %zu",	stat->p_small, stat->b_small);	
	info("exact slab use page : %zu,\tbytes : %zu",	stat->p_exact, stat->b_exact);
	info("big   slab use page : %zu,\tbytes : %zu",	stat->p_big,   stat->b_big);	
	info("page slab use page  : %zu,\tbytes : %zu\n",	stat->p_page,  stat->b_page);				

	info("max free pages : %zu\n",		stat->max_free_pages);
}

static bool 
ncx_slab_empty(ncx_slab_pool_t *pool, ncx_slab_page_t *page)
{
	ncx_slab_page_t *prev;
	
	if (page->slab == 0) {
		return true;
	}

	//page->prev == PAGE | SMALL | EXACT | BIG
	if (page->next == NULL ) {
		return false;
	}

	prev = (ncx_slab_page_t *)(page->prev & ~NCX_SLAB_PAGE_MASK);   
	while (prev >= pool->pages) { 
		prev = (ncx_slab_page_t *)(prev->prev & ~NCX_SLAB_PAGE_MASK);   
	};

	if (prev == &pool->free) {
		return true;
	}

	return false;
}

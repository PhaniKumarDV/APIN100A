/*
 * Copyright (c) 2002-2014, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @defgroup adf_nbuf_public network buffer API
 */

/**
 * @ingroup adf_nbuf_public
 * @file adf_nbuf.h
 * This file defines the network buffer abstraction.
 */

#ifndef _ADF_NBUF_H
#define _ADF_NBUF_H
#ifdef ATHR_WIN_NWF
#pragma warning( disable:4214 ) //bit field types other than int
#endif
#include <adf_os_util.h>
#include <adf_os_types.h>
#include <adf_os_dma.h>
#include <adf_net_types.h>
#include <adf_nbuf_pvt.h>

/**
 * @brief Platform indepedent packet abstraction
 */
typedef __adf_nbuf_t 	adf_nbuf_t;

/**
 * @brief Dma map callback prototype
 */
typedef void (*adf_os_dma_map_cb_t)(void *arg, adf_nbuf_t buf,
                                    adf_os_dma_map_t dmap);

/**
 * @brief invalid handle
 */
#define ADF_NBUF_NULL   __ADF_NBUF_NULL
/**
 * @brief Platform independent packet queue abstraction
 */
typedef __adf_nbuf_queue_t   adf_nbuf_queue_t;

/**
 * BUS/DMA mapping routines
 */

/**
 * @brief Create a DMA map. This can later be used to map
 *        networking buffers. They :
 *          - need space in adf_drv's software descriptor
 *          - are typically created during adf_drv_create
 *          - need to be created before any API(adf_nbuf_map) that uses them
 *
 * @param[in]  osdev os device
 * @param[out] dmap  map handle
 *
 * @return status of the operation
 */
static inline a_status_t
adf_nbuf_dmamap_create(adf_os_device_t osdev,
                       adf_os_dma_map_t *dmap)
{
    return (__adf_nbuf_dmamap_create(osdev, dmap));
}


/**
 * @brief Delete a dmap map
 *
 * @param[in] osdev os device
 * @param[in] dmap
 */
static inline void
adf_nbuf_dmamap_destroy(adf_os_device_t osdev, adf_os_dma_map_t dmap)
{
    __adf_nbuf_dmamap_destroy(osdev, dmap);
}

/**
 * @brief Setup the map callback for a dma map
 *
 * @param[in] map DMA map
 * @param[in] cb  callback function
 * @param[in] arg context for callback function
 */
static inline void
adf_nbuf_dmamap_set_cb(adf_os_dma_map_t dmap, adf_os_dma_map_cb_t cb,
                       void *arg)
{
    __adf_nbuf_dmamap_set_cb(dmap, cb, arg);
}

/**
 * @brief Map a buffer to local bus address space
 *
 * @param[in] osdev  os device
 * @param[in] buf    buf to be mapped
 *                   (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    DMA direction
 *
 * @return status of the operation
 */
static inline a_status_t
adf_nbuf_map(adf_os_device_t        osdev,
             adf_nbuf_t             buf,
             adf_os_dma_dir_t       dir)
{
#if defined(HIF_PCI)
    return __adf_nbuf_map(osdev, buf, dir);
#else
    return 0;
#endif
}

static inline void
adf_nbuf_set_sendcompleteflag(adf_nbuf_t buf, a_bool_t flag)
{
    __adf_nbuf_set_sendcompleteflag(buf, flag);
}
/**
 * @brief Map a buffer to local bus address space
 *
 * @param[in] osdev  os device
 * @param[in] buf    buf to be mapped
 *                   (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    DMA direction
 * @param[in] nbytes No of Bytes to be mapped
 *
 * @return status of the operation
 */
static inline a_status_t
adf_nbuf_map_nbytes(adf_os_device_t        osdev,
             adf_nbuf_t             buf,
             adf_os_dma_dir_t       dir,
             int nbytes )
{
#if defined(HIF_PCI)
    return __adf_nbuf_map_nbytes(osdev, buf, dir, nbytes);
#else
    return 0;
#endif
}


/**
 * @brief Unmap a previously mapped buf
 *
 * @param[in] osdev  os device
 * @param[in] buf    buf to be unmapped
 *                   (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    DMA direction
 */
static inline void
adf_nbuf_unmap(adf_os_device_t      osdev,
               adf_nbuf_t           buf,
               adf_os_dma_dir_t     dir)
{
#if defined(HIF_PCI)
    __adf_nbuf_unmap(osdev, buf, dir);
#endif
}

#ifndef REMOVE_INIT_DEBUG_CODE
/**
 * @brief sync for CPU
 *
 * @param[in] osdev  os device
 * @param[in] buf    buf to be unmapped
 *                   (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    DMA direction
 */
static inline void
adf_nbuf_sync_for_cpu(adf_os_device_t      osdev,
               adf_nbuf_t           buf,
               adf_os_dma_dir_t     dir)
{
#if defined(HIF_PCI)
    __adf_nbuf_sync_for_cpu(osdev, buf, dir);
#endif
}
#endif

/**
 * @brief Unmap a previously mapped buf
 *
 * @param[in] osdev  os device
 * @param[in] buf    buf to be unmapped
 *                   (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    DMA direction
 * @param[in] nbytes No of bytes to be unmapeed
 *
 */
static inline void
adf_nbuf_unmap_nbytes(adf_os_device_t      osdev,
               adf_nbuf_t           buf,
               adf_os_dma_dir_t     dir,
               int nbytes )
{
#if defined(HIF_PCI)
    __adf_nbuf_unmap_nbytes(osdev, buf, dir, nbytes);
#endif
}



static inline a_status_t
adf_nbuf_map_single(
    adf_os_device_t osdev, adf_nbuf_t buf, adf_os_dma_dir_t dir)
{
#if defined(HIF_PCI)
    return __adf_nbuf_map_single(osdev, buf, dir);
#else
    return 0;
#endif
}

static inline a_status_t
adf_nbuf_map_nbytes_single(
    adf_os_device_t osdev, adf_nbuf_t buf, adf_os_dma_dir_t dir, int nbytes)
{
#if defined(HIF_PCI)
    return __adf_nbuf_map_nbytes_single(osdev, buf, dir, nbytes);
#else
    return 0;
#endif
}

static inline void
adf_nbuf_unmap_single(
    adf_os_device_t osdev, adf_nbuf_t buf, adf_os_dma_dir_t dir)
{
#if defined(HIF_PCI)
    __adf_nbuf_unmap_single(osdev, buf, dir);
#endif
}

static inline void
adf_nbuf_unmap_nbytes_single(
    adf_os_device_t osdev, adf_nbuf_t buf, adf_os_dma_dir_t dir, int nbytes)
{
#if defined(HIF_PCI)
    __adf_nbuf_unmap_nbytes_single(osdev, buf, dir, nbytes);
#endif
}

static inline int
adf_nbuf_get_num_frags(adf_nbuf_t buf)
{
    return __adf_nbuf_get_num_frags(buf);
}

static inline int
adf_nbuf_get_frag_len(adf_nbuf_t buf, int frag_num)
{
    return __adf_nbuf_get_frag_len(buf, frag_num);
}

static inline unsigned char *
adf_nbuf_get_frag_vaddr(adf_nbuf_t buf, int frag_num)
{
    return __adf_nbuf_get_frag_vaddr(buf, frag_num);
}

static inline unsigned char *
adf_nbuf_get_frag_vaddr_always(adf_nbuf_t buf)
{
    return __adf_nbuf_get_frag_vaddr_always(buf);
}

static inline a_uint32_t
adf_nbuf_get_frag_paddr_lo(adf_nbuf_t buf, int frag_num)
{
    return __adf_nbuf_get_frag_paddr_lo(buf, frag_num);
}

static inline a_uint32_t
adf_nbuf_get_frag_paddr_0(adf_nbuf_t buf)
{
    return __adf_nbuf_get_frag_paddr_0(buf);
}

static inline int
adf_nbuf_get_frag_is_wordstream(adf_nbuf_t buf, int frag_num)
{
    return __adf_nbuf_get_frag_is_wordstream(buf, frag_num);
}

static inline void
adf_nbuf_set_frag_is_wordstream(adf_nbuf_t buf, int frag_num, int is_wordstream)
{
    __adf_nbuf_set_frag_is_wordstream(buf, frag_num, is_wordstream);
}

static inline void
adf_nbuf_set_vdev_ctx(adf_nbuf_t buf, a_int32_t vdev_ctx)
{
    __adf_nbuf_set_vdev_ctx(buf, vdev_ctx);
}

static inline void
adf_nbuf_set_fctx_type(adf_nbuf_t buf, a_int32_t ctx, a_int8_t type)
{
    __adf_nbuf_set_fctx_type(buf, ctx, type);
}

static inline a_uint32_t
adf_nbuf_get_vdev_ctx(adf_nbuf_t buf)
{
    return  __adf_nbuf_get_vdev_ctx(buf);
}

static inline a_uint32_t
adf_nbuf_get_fctx(adf_nbuf_t buf)
{
    return  __adf_nbuf_get_fctx(buf);
}

static inline a_uint8_t
adf_nbuf_get_ftype(adf_nbuf_t buf)
{
    return  __adf_nbuf_get_ftype(buf);
}

static inline void
adf_nbuf_frag_push_head(
    adf_nbuf_t buf,
    int frag_len,
    char *frag_vaddr,
    u_int32_t frag_paddr_lo,
    u_int32_t frag_paddr_hi)
{
    __adf_nbuf_frag_push_head(
        buf, frag_len, frag_vaddr, frag_paddr_lo, frag_paddr_hi);
}

/* For efficiency, it is the responsibility of the caller to ensure that val
 * is either 0 or 1.
 */
static inline void
adf_nbuf_set_chfrag_start(adf_nbuf_t buf, u_int8_t val)
{
    __adf_nbuf_set_chfrag_start(buf, val);
}

static inline int
adf_nbuf_is_chfrag_start(adf_nbuf_t buf)
{
    return __adf_nbuf_is_chfrag_start(buf);
}

/* For efficiency, it is the responsibility of the caller to ensure that val
 * is either 0 or 1.
 */
static inline void
adf_nbuf_set_chfrag_end(adf_nbuf_t buf, u_int8_t val)
{
    __adf_nbuf_set_chfrag_end(buf, val);
}

static inline int
adf_nbuf_is_chfrag_end(adf_nbuf_t buf)
{
    return __adf_nbuf_is_chfrag_end(buf);
}

/**
 * @brief returns information about the mapped buf
 *
 * @param[in]  bmap map handle
 * @param[out] sg   map info
 */
static inline void
adf_nbuf_dmamap_info(adf_os_dma_map_t bmap, adf_os_dmamap_info_t *sg)
{
    __adf_nbuf_dmamap_info(bmap, sg);
}



/*
 * nbuf allocation rouines
 */


/**
 * @brief Allocate adf_nbuf
 *
 * The nbuf created is guarenteed to have only 1 physical segment
 *
 * @param[in] hdl   platform device object
 * @param[in] size  data buffer size for this adf_nbuf including max header
 *                  size
 * @param[in] reserve  headroom to start with.
 * @param[in] align    alignment for the start buffer.
 * @param[i] prio   Indicate if the nbuf is high priority (some OSes e.g darwin
 *                   polls few times if allocation fails and priority is  TRUE)
 *
 * @return The new adf_nbuf instance or NULL if there's not enough memory.
 */
static inline adf_nbuf_t
adf_nbuf_alloc(adf_os_device_t      osdev,
               adf_os_size_t        size,
               int                  reserve,
               int                  align,
               int                  prio)
{
    return __adf_nbuf_alloc(osdev, size, reserve,align, prio);
}


/**
 * @brief Free adf_nbuf
 *
 * @param[in] buf buffer to free
 */
static inline void
adf_nbuf_free(adf_nbuf_t buf)
{
    __adf_nbuf_free(buf);
}

/**
 * @brief Free adf_nbuf
 *
 * @param[in] buf buffer to free
 */
static inline void
adf_nbuf_ref(adf_nbuf_t buf)
{
    __adf_nbuf_ref(buf);
}

/**
 *  @brief Check whether the buffer is shared
 *  @param skb: buffer to check
 *
 *  Returns true if more than one person has a reference to this
 *  buffer.
 */
static inline int
adf_nbuf_shared(adf_nbuf_t buf)
{
    return __adf_nbuf_shared(buf);
}


/**
 * @brief Free a list of adf_nbufs and tell the OS their tx status (if req'd)
 *
 * @param[in] bufs - list of netbufs to free
 * @param[in] tx_err - whether the tx frames were transmitted successfully
 */
static inline void
adf_nbuf_tx_free(adf_nbuf_t buf_list, int tx_err)
{
    __adf_nbuf_tx_free(buf_list, tx_err);
}

/**
 * @brief Reallocate such that there's required headroom in
 *        buf. Note that this can allocate a new buffer, or
 *        change geometry of the orignial buffer. The new buffer
 *        is returned in the (new_buf).
 *
 * @param[in] buf (older buffer)
 * @param[in] headroom
 *
 * @return newly allocated buffer
 */
static inline adf_nbuf_t
adf_nbuf_realloc_headroom(adf_nbuf_t buf, a_uint32_t headroom)
{
    return (__adf_nbuf_realloc_headroom(buf, headroom));
}


/**
 * @brief expand the tailroom to the new tailroom, but the buffer
 * remains the same
 *
 * @param[in] buf       buffer
 * @param[in] tailroom  new tailroom
 *
 * @return expanded buffer or NULL on failure
 */
static inline adf_nbuf_t
adf_nbuf_realloc_tailroom(adf_nbuf_t buf, a_uint32_t tailroom)
{
    return (__adf_nbuf_realloc_tailroom(buf, tailroom));
}


/**
 * @brief this will expand both tail & head room for a given
 *        buffer, you may or may not get a new buffer.Use it
 *        only when its required to expand both. Otherwise use
 *        realloc (head/tail) will solve the purpose. Reason for
 *        having an extra API is that some OS do this in more
 *        optimized way, rather than calling realloc (head/tail)
 *        back to back.
 *
 * @param[in] buf       buffer
 * @param[in] headroom  new headroom
 * @param[in] tailroom  new tailroom
 *
 * @return expanded buffer
 */
static inline adf_nbuf_t
adf_nbuf_expand(adf_nbuf_t buf, a_uint32_t headroom, a_uint32_t tailroom)
{
    return (__adf_nbuf_expand(buf,headroom,tailroom));
}


/**
 * @brief Copy src buffer into dst. This API is useful, for
 *        example, because most native buffer provide a way to
 *        copy a chain into a single buffer. Therefore as a side
 *        effect, it also "linearizes" a buffer (which is
 *        perhaps why you'll use it mostly). It creates a
 *        writeable copy.
 *
 * @param[in] buf source nbuf to copy from
 *
 * @return the new nbuf
 */
static inline adf_nbuf_t
adf_nbuf_copy(adf_nbuf_t buf)
{
    return(__adf_nbuf_copy(buf));
}


/**
 * @brief link two nbufs, the new buf is piggybacked into the
 *        older one.
 *
 * @param[in] dst   buffer to piggyback into
 * @param[in] src   buffer to put
 *
 * @return status of the call - 0 successful
 */
static inline a_status_t
adf_nbuf_cat(adf_nbuf_t dst, adf_nbuf_t src)
{
    return __adf_nbuf_cat(dst, src);
}


/**
 * @brief return the length of the copy bits for skb
 *
 * @param skb, offset, len, to
 *
 * @return int32_t
 */
static inline int32_t
adf_nbuf_copy_bits(adf_nbuf_t nbuf, u_int32_t offset, u_int32_t len, void *to)
{
    return __adf_nbuf_copy_bits(nbuf, offset, len, to);
}


/**
 * @brief clone the nbuf (copy is readonly)
 *
 * @param[in] buf nbuf to clone from
 *
 * @return cloned buffer
 */
static inline adf_nbuf_t
adf_nbuf_clone(adf_nbuf_t buf)
{
    return(__adf_nbuf_clone(buf));
}


/**
 * @brief  Create a version of the specified nbuf whose
 *         contents can be safely modified without affecting
 *         other users.If the nbuf is a clone then this function
 *         creates a new copy of the data. If the buffer is not
 *         a clone the original buffer is returned.
 *
 * @param[in] buf   source nbuf to create a writable copy from
 *
 * @return new buffer which is writeable
 */
static inline adf_nbuf_t
adf_nbuf_unshare(adf_nbuf_t buf)
{
    return(__adf_nbuf_unshare(buf));
}



/*
 * nbuf manipulation routines
 */

/**
 * @brief  return the address of an nbuf's buffer
 *
 * @param[in] buf netbuf
 *
 * @return head address
 */
static inline a_uint8_t *
adf_nbuf_head(adf_nbuf_t buf)
{
    return __adf_nbuf_head(buf);
}

/**
 * @brief  return the address of the start of data within an nbuf
 *
 * @param[in] buf buffer
 *
 * @return data address
 */
static inline a_uint8_t *
adf_nbuf_data(adf_nbuf_t buf)
{
    return __adf_nbuf_data(buf);
}


/**
 * @brief return the amount of headroom int the current nbuf
 *
 * @param[in] buf   buffer
 *
 * @return amount of head room
 */
static inline a_uint32_t
adf_nbuf_headroom(adf_nbuf_t buf)
{
    return (__adf_nbuf_headroom(buf));
}


/**
 * @brief return the amount of tail space available
 *
 * @param[in] buf   buffer
 *
 * @return amount of tail room
 */
static inline a_uint32_t
adf_nbuf_tailroom(adf_nbuf_t buf)
{
    return (__adf_nbuf_tailroom(buf));
}


/**
 * @brief Push data in the front
 *
 * @param[in] buf      buf instance
 * @param[in] size     size to be pushed
 *
 * @return New data pointer of this buf after data has been pushed,
 *         or NULL if there is not enough room in this buf.
 */
static inline a_uint8_t *
adf_nbuf_push_head(adf_nbuf_t buf, adf_os_size_t size)
{
    return __adf_nbuf_push_head(buf, size);
}


/**
 * @brief Puts data in the end
 *
 * @param[in] buf      buf instance
 * @param[in] size     size to be pushed
 *
 * @return data pointer of this buf where new data has to be
 *         put, or NULL if there is not enough room in this buf.
 */
static inline a_uint8_t *
adf_nbuf_put_tail(adf_nbuf_t buf, adf_os_size_t size)
{
    return __adf_nbuf_put_tail(buf, size);
}


/**
 * @brief pull data out from the front
 *
 * @param[in] buf   buf instance
 * @param[in] size     size to be popped
 *
 * @return New data pointer of this buf after data has been popped,
 *         or NULL if there is not sufficient data to pull.
 */
static inline a_uint8_t *
adf_nbuf_pull_head(adf_nbuf_t buf, adf_os_size_t size)
{
    return __adf_nbuf_pull_head(buf, size);
}


/**
 *
 * @brief trim data out from the end
 *
 * @param[in] buf   buf instance
 * @param[in] size     size to be popped
 *
 * @return none
 */
static inline void
adf_nbuf_trim_tail(adf_nbuf_t buf, adf_os_size_t size)
{
    __adf_nbuf_trim_tail(buf, size);
}


/**
 * @brief Get the length of the buf
 *
 * @param[in] buf the buf instance
 *
 * @return The total length of this buf.
 */
static inline adf_os_size_t
adf_nbuf_len(adf_nbuf_t buf)
{
    return (__adf_nbuf_len(buf));
}

/**
 * @brief Set the length of the buf
 *
 * @param[in] buf the buf instance
 * @param[in] size to be set
 *
 * @return none
 */
static inline void
adf_nbuf_set_pktlen(adf_nbuf_t buf, uint32_t len)
{
    __adf_nbuf_set_pktlen(buf, len);
}

/**
 * @brief test whether the nbuf is cloned or not
 *
 * @param[in] buf   buffer
 *
 * @return TRUE if it is cloned, else FALSE
 */
static inline a_bool_t
adf_nbuf_is_cloned(adf_nbuf_t buf)
{
    return (__adf_nbuf_is_cloned(buf));
}

/**
 *
 * @brief trim data out from the end
 *
 * @param[in] buf   buf instance
 * @param[in] size     size to be popped
 *
 * @return none
 */
static inline void
adf_nbuf_reserve(adf_nbuf_t buf, adf_os_size_t size)
{
    __adf_nbuf_reserve(buf, size);
}


/*
 * nbuf frag routines
 */

/**
 * @brief return the frag pointer & length of the frag
 *
 * @param[in]  buf   buffer
 * @param[out] sg    this will return all the frags of the nbuf
 *
 */
static inline void
adf_nbuf_frag_info(adf_nbuf_t buf, adf_os_sglist_t *sg)
{
    __adf_nbuf_frag_info(buf, sg);
}
/**
 * @brief return the data pointer & length of the header
 *
 * @param[in]  buf  nbuf
 * @param[out] addr data pointer
 * @param[out] len  length of the data
 *
 */
static inline void
adf_nbuf_peek_header(adf_nbuf_t buf, a_uint8_t **addr, a_uint32_t *len)
{
    __adf_nbuf_peek_header(buf, addr, len);
}
/*
 * nbuf private context routines
 */

/**
 * @brief get the priv pointer from the nbuf'f private space
 *
 * @param[in] buf
 *
 * @return data pointer to typecast into your priv structure
 */
static inline a_uint8_t *
adf_nbuf_get_priv(adf_nbuf_t buf)
{
    return (__adf_nbuf_get_priv(buf));
}


/*
 * nbuf queue routines
 */


/**
 * @brief Initialize buf queue
 *
 * @param[in] head  buf queue head
 */
static inline void
adf_nbuf_queue_init(adf_nbuf_queue_t *head)
{
    __adf_nbuf_queue_init(head);
}

/**
 * @brief Append src list at the end of dest list
 *
 * @param[in] dest target netbuf queue
 * @param[in] src  source netbuf queue
 */
static inline adf_nbuf_queue_t *
adf_nbuf_queue_append(adf_nbuf_queue_t *dest, adf_nbuf_queue_t *src)
{
    return (__adf_nbuf_queue_append(dest, src));
}

/**
 * @brief free a queue
 *
 * @param qhead
 */
static inline void
adf_nbuf_queue_free(adf_nbuf_queue_t *head)
{
    __adf_nbuf_queue_free(head);
}


/**
 * @brief Append a nbuf to the tail of the buf queue
 *
 * @param[in] head  buf queue head
 * @param[in] buf   buf
 */
static inline void
adf_nbuf_queue_add(adf_nbuf_queue_t *head, adf_nbuf_t buf)
{
    __adf_nbuf_queue_add(head, buf);
}

/**
 * @brief   Insert nbuf at the head of queue
 *
 * @param[in] head  buf queue head
 * @param[in] buf   buf
 */
static inline void
adf_nbuf_queue_insert_head(adf_nbuf_queue_t *head, adf_nbuf_t buf)
{
    __adf_nbuf_queue_insert_head(head, buf);
}



/**
 * @brief Retrieve a buf from the head of the buf queue
 *
 * @param[in] head    buf queue head
 *
 * @return The head buf in the buf queue.
 */
static inline adf_nbuf_t
adf_nbuf_queue_remove(adf_nbuf_queue_t *head)
{
    return __adf_nbuf_queue_remove(head);
}


/**
 * @brief get the length of the queue
 *
 * @param[in] head  buf queue head
 *
 * @return length of the queue
 */
static inline a_uint32_t
adf_nbuf_queue_len(adf_nbuf_queue_t *head)
{
    return __adf_nbuf_queue_len(head);
}


/**
 * @brief get the first guy/packet in the queue
 *
 * @param[in] head  buf queue head
 *
 * @return first buffer in queue
 */
static inline adf_nbuf_t
adf_nbuf_queue_first(adf_nbuf_queue_t *head)
{
    return (__adf_nbuf_queue_first(head));
}


/**
 * @brief get the next guy/packet of the given buffer (or
 *        packet)
 *
 * @param[in] buf   buffer
 *
 * @return next buffer/packet
 */
static inline adf_nbuf_t
adf_nbuf_queue_next(adf_nbuf_t buf)
{
    return (__adf_nbuf_queue_next(buf));
}


/**
 * @brief Check if the buf queue is empty
 *
 * @param[in] nbq   buf queue handle
 *
 * @return    TRUE  if queue is empty
 * @return    FALSE if queue is not emty
 */
static inline a_bool_t
adf_nbuf_is_queue_empty(adf_nbuf_queue_t * nbq)
{
    return __adf_nbuf_is_queue_empty(nbq);
}


/**
 * @brief get the next packet in the linked list
 * @details
 *  This function can be used when nbufs are directly linked into a list,
 *  rather than using a separate network buffer queue object.
 *
 * @param[in] buf buffer
 *
 * @return next network buffer in the linked list
 */
static inline adf_nbuf_t
adf_nbuf_next(adf_nbuf_t buf)
{
    return __adf_nbuf_next(buf);
}


/**
 * @brief add a packet to a linked list
 * @details
 *  This function can be used to directly link nbufs, rather than using
 *  a separate network buffer queue object.
 *
 * @param[in] this_buf predecessor buffer
 * @param[in] next_buf successor buffer
 */
static inline void
adf_nbuf_set_next(adf_nbuf_t this_buf, adf_nbuf_t next_buf)
{
    __adf_nbuf_set_next(this_buf, next_buf);
}


/*
 * nbuf extension routines XXX
 */

/**
 * @brief link extension of this packet contained in a new nbuf
 * @details
 *  This function is used to link up many nbufs containing a single logical
 *  packet - not a collection of packets. Do not use for linking the first
 *  extension to the head
 * @param[in] this_buf predecessor buffer
 * @param[in] next_buf successor buffer
 */
static inline void
adf_nbuf_set_next_ext(adf_nbuf_t this_buf, adf_nbuf_t next_buf)
{
    __adf_nbuf_set_next_ext(this_buf, next_buf);
}

/**
 * @brief get the next packet extension in the linked list
 * @details
 *
 * @param[in] buf buffer
 *
 * @return next network buffer in the linked list
 */
static inline adf_nbuf_t
adf_nbuf_next_ext(adf_nbuf_t buf)
{
    return __adf_nbuf_next_ext(buf);
}

/**
 * @brief link list of packet extensions to the head segment
 * @details
 *  This function is used to link up a list of packet extensions (seg1, 2,
 *  ...) to the nbuf holding the head segment (seg0)
 * @param[in] head_buf nbuf holding head segment (single)
 * @param[in] ext_list nbuf list holding linked extensions to the head
 * @param[in] ext_len Total length of all buffers in the extension list
 */
static inline void
adf_nbuf_append_ext_list(adf_nbuf_t head_buf, adf_nbuf_t ext_list,
        adf_os_size_t ext_len)
{
    __adf_nbuf_append_ext_list(head_buf, ext_list, ext_len);
}

/**
 * @brief Gets the tx checksumming to be performed on this buf
 *
 * @param[in]  buf       buffer
 * @param[out] hdr_off   the (tcp) header start
 * @param[out] where     the checksum offset
 */
static inline adf_net_cksum_type_t
adf_nbuf_tx_cksum_info(adf_nbuf_t buf, a_uint8_t **hdr_off, a_uint8_t **where)
{
    return(__adf_nbuf_tx_cksum_info(buf, hdr_off, where));
}


/**
 * @brief Gets the tx checksum offload demand
 *
 * @param[in]  buf             buffer
 * @return adf_nbuf_tx_cksum_t checksum offload demand for the frame
 */
static inline adf_nbuf_tx_cksum_t
adf_nbuf_get_tx_cksum(adf_nbuf_t buf)
{
    return (__adf_nbuf_get_tx_cksum(buf));
}

/**
 * @brief Drivers that support hw checksumming use this to
 *        indicate checksum info to the stack.
 *
 * @param[in]  buf      buffer
 * @param[in]  cksum    checksum
 */
static inline void
adf_nbuf_set_rx_cksum(adf_nbuf_t buf, adf_nbuf_rx_cksum_t *cksum)
{
    __adf_nbuf_set_rx_cksum(buf, cksum);
}


/**
 * @brief Drivers that are capable of TCP Large segment offload
 *        use this to get the offload info out of an buf.
 *
 * @param[in]  buf  buffer
 * @param[out] tso  offload info
 */
static inline void
adf_nbuf_get_tso_info(adf_nbuf_t buf, adf_nbuf_tso_t *tso)
{
    __adf_nbuf_get_tso_info(buf, tso);
}


/*static inline void
adf_nbuf_set_vlan_info(adf_nbuf_t buf, adf_net_vlan_tag_t vlan_tag)
{
    __adf_nbuf_set_vlan_info(buf, vlan_tag);
}*/

/**
 * @brief This function extracts the vid & priority from an
 *        nbuf
 *
 *
 * @param[in] hdl   net handle
 * @param[in] buf   buffer
 * @param[in] vlan  vlan header
 *
 * @return status of the operation
 */
static inline a_status_t
adf_nbuf_get_vlan_info(adf_net_handle_t hdl, adf_nbuf_t buf,
                       adf_net_vlanhdr_t *vlan)
{
    return __adf_nbuf_get_vlan_info(hdl, buf, vlan);
}

/**
 * @brief This function extracts the TID value from nbuf
 *
 * @param[in] buf   buffer
 *
 * @return TID value
 */
static inline a_uint8_t
adf_nbuf_get_tid(adf_nbuf_t buf)
{
    return __adf_nbuf_get_tid(buf);
}

static inline void
adf_nbuf_reset_ctxt(__adf_nbuf_t nbuf)
{
    __adf_nbuf_reset_ctxt(nbuf);
}

static inline void
adf_nbuf_set_rx_info(__adf_nbuf_t nbuf, void * info, a_uint32_t len, u_int8_t offset)
{
    __adf_nbuf_set_rx_info(nbuf, info, len, offset);
}

static inline void *
adf_nbuf_get_rx_info(__adf_nbuf_t nbuf, u_int8_t offset)
{
    return (__adf_nbuf_get_rx_info(nbuf, offset));
}

/**
 * @brief Nbuf initialization routine frags num field
 * @details
 *  This function is used to init skb->cb.extra_frags.num to 0
 * @param[in] nbuf  buffer
 * @return void
 */
#define adf_nbuf_frags_num_init(_nbuf) __adf_nbuf_num_frags_init((_nbuf))

/**
 * @brief Nbuf initialization routine
 * @details
 *  This function is used to re-init nbufs.
 *  The purpose is to re-use the nbufs, instead of freeing it
 *  and re-allocating them again. Re-use of nbufs contributes
 *  to saving critical CPU cycles on CPU bound host.
 * @param[in] nbuf  buffer
 * @return void
 */
#define adf_nbuf_init(_nbuf) __adf_nbuf_init((_nbuf))

/**
 *
 * @brief extract network header of the packet
 *
 * @return pointer to network header located insidethe buffer
 **/
static inline void *
adf_nbuf_network_header(adf_nbuf_t buf)
{
    return(__adf_nbuf_network_header(buf));
}
/**
 *
 * @brief extract transport header of the packet
 *
 * @return pointer to transport header located insidethe buffer
 * */
static inline void *
adf_nbuf_transport_header(adf_nbuf_t buf)
{
    return(__adf_nbuf_transport_header(buf));
}


/**
 *  * @brief return the size of TCP segment size (MSS),
 *   * passed as part of network buffer by network stack
 *
 *    *
 *     * @param[in] buf the buf instance
 *      *
 *       * @return The MSS value
 *        */
static inline adf_os_size_t
adf_nbuf_tcp_tso_size(adf_nbuf_t buf)
{
    return (__adf_nbuf_tcp_tso_size(buf));
}
/**
 * @brief is gso enabled for this particular network buffer
 * based on info. passed from network stack
 *
 * @return true or false
 **/
static inline a_bool_t
adf_nbuf_is_tso(adf_nbuf_t buf)
{
    return (__adf_nbuf_is_tso(buf));
}

/**
 * @brief return parent skb of the TCP Segment(cloned from original one),
 * parent is stored as part of skb->cb
 *
 * @param[in] buf the buf instance
 * @return parent skb
 **/
static inline adf_nbuf_t
adf_nbuf_get_parent(adf_nbuf_t buf)
{
    return (__adf_nbuf_get_parent(buf));
}

/**
 *  * @brief store parent skb part of the TCP Segment(cloned from original one),
 *  parent is stored as part of skb->cb
 *
 * @param[in] buf the buf instance
 *
 * @return none
 **/
static inline void
adf_nbuf_put_parent(adf_nbuf_t buf,adf_nbuf_t parent)
{
    __adf_nbuf_put_parent(buf,parent);
}


/**
 *  * @brief returns a pointer the cb array of skb
 *  @details This pointer is typecast to a lro_info
 *  structure and copy the HW LRO specific info
 *  from rx descriptor which is later used by
 *  SW LRO moddule.
 *  @param[in] nbuf  buffer
 *  @return (void *) pointer to cb of skb
 * */
static inline void *
adf_nbuf_get_cb(adf_nbuf_t nbuf)
{
    return __adf_nbuf_get_cb(nbuf);
}

/**
 * * @brief return the number of fragments in the skb
 * @param(in) nbuf buffer
 * @return number of fragments of the skb
 */
static inline a_uint32_t
adf_nbuf_get_nr_frags(adf_nbuf_t nbuf)
{
    return __adf_nbuf_get_nr_frags(nbuf);
}

/**
 * @brief return the size of the linear buffer of the
 * network buffer
 *
 * @param[in] buf the buf instance
 * @return The linear buffer size
 **/
static inline adf_os_size_t
adf_nbuf_headlen(adf_nbuf_t buf)
{
    return (__adf_nbuf_headlen(buf));
}


/**
 * @brief Map a buffer to local bus address space
 *
 * @param[in] osdev  	os device
 * @param[in] buf    	buf whose fragment is mapped
 * @param[in] offset 	offset from where we need to map the fragment
 *  (mapping info is stored in the buf's meta-data area)
 * @param[in] dir    	DMA direction
 * @param[in] cur_frag 	fragment to be mapped
 *
 * @return status of the operation
 **/

static inline a_status_t
adf_nbuf_frag_map(adf_os_device_t        osdev,
             adf_nbuf_t             buf,
             int offset,
             adf_os_dma_dir_t       dir,
             int cur_frag)
{
#if defined(HIF_PCI)
    return __adf_nbuf_frag_map(osdev, buf, offset, dir, cur_frag);
#else
    return 0;
#endif
}

/**
 * @brief to check if the TSO TCP pkt is a IPv4 or not.
 *
 * @param[in]  buf  buffer
 * @return     True(1) if IPv4 TCP pkt or else false(0)
 **/
static inline a_bool_t
adf_nbuf_tso_tcp_v4(adf_nbuf_t buf)
{
    return __adf_nbuf_tso_tcp_v4(buf);
}

/**
 * @brief to check if the TSO TCP pkt is a IPv6 or not.
 *
 * @param[in]  buf  buffer
 * @return     True(1) if IPv6 TCP pkt or else false(0)
 **/
static inline a_bool_t
adf_nbuf_tso_tcp_v6(adf_nbuf_t buf)
{
    return __adf_nbuf_tso_tcp_v6(buf);
}

/**
 * @brief get the TCP sequence number from TCP header of the nbuf
 *
 * @param[in]  buf  buffer
 * @return     TCP sequence number
 **/
static inline a_uint32_t
adf_nbuf_tcp_seq(adf_nbuf_t buf)
{
    return __adf_nbuf_tcp_seq(buf);
}

/**
 * @brief Get the l2+l3+l4 hdr length of
 * the buf
 *
 * @param[in] buf the buf instance
 *
 * @return The l2+l3+l4 hdr length of
 * this buf.
 **/
static inline adf_os_size_t
adf_nbuf_l2l3l4_hdr_len(adf_nbuf_t buf)
{
    return (__adf_nbuf_l2l3l4_hdr_len(buf));
}

/**
 * @brief is gso enabled for this particular network buffer
 * based on info. passed from network stack
 *
 * @return true or false
 **/
static inline a_bool_t
adf_nbuf_is_nonlinear(adf_nbuf_t buf)
{
    return (__adf_nbuf_is_nonlinear(buf));
}

/**
 * @brief get the size of a fragment  in the network buffer
 *
 * @param[in]  buf  buffer
 * @param[in]  fragment number
 * @return     fragment size
 **/
static inline a_uint32_t
adf_nbuf_get_frag_size(adf_nbuf_t buf, a_uint32_t frag_num)
{
    return __adf_nbuf_get_frag_size(buf, frag_num);
}

/*
 * @brief return the priority value of the skb
 *
 * @param skb
 *
 * @return uint32_t
 */
static inline uint32_t
adf_nbuf_get_priority(adf_nbuf_t buf)
{
    return __adf_nbuf_get_priority(buf);
}

/*
 * @brief set the priority value of the skb
 *
 * @param skb, priority
 *
 * @return void
 */
static inline void
adf_nbuf_set_priority(adf_nbuf_t buf, uint32_t p)
{
    __adf_nbuf_set_priority(buf, p);
}

/**
 * @brief get the queue mapping from the nbuf
 *
 * @param[in]  buffer
 * @return     queue mapping
 **/
static inline a_uint16_t
adf_nbuf_get_queue_mapping(adf_nbuf_t buf)
{
    return __adf_nbuf_get_queue_mapping(buf);
}

#endif

/*
 * Copyright (c) 2026 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @ingroup zrpc_tlv
 * @brief zRPC generic TLV header.
 */

#ifndef ZEPHYR_INCLUDE_RPC_ZRPC_TLV_H_
#define ZEPHYR_INCLUDE_RPC_ZRPC_TLV_H_

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/rpc/zrpc.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zRPC TLV implementation.
 * @defgroup zrpc_tlvb zRPC binary format
 * @ingroup zrpc
 * @{
 */

/** @cond PRIVATE */

#ifdef __cplusplus
#define zrpc_tlv_restrict__
#else
#define zrpc_tlv_restrict__ restrict
#endif

/** @endcond */

/** Attribute alignment boundary */
#define zrpc_alignto alignof(struct zrpc_attr)

/** Align provided value @c v up to next multiple of @c zrpc_alingto */
#define zrpc_align(v)	(((v) + zrpc_alignto - 1u) & ~(zrpc_alignto - 1u))


/** zRPC TLV buffer */
struct zrpc_tlvb {
	/** Index of the buffer head */
	uint32_t head;
	/** Buffer capacity */
	uint32_t capacity;
	/** Address of the buffer */
	uint8_t *buf;
};

/**
 * @brief Initialize zRPC TLV buffer.
 *
 * @param tlvb     Address of the TLV buffer abstractionq
 * @param capacity Capacity of the buffer at @c buf.
 * @param buf      Address of the underlying buffer.
 */
inline void zrpc_tlvb_init(struct zrpc_tlvb *zrpc_tlv_restrict__ tlvb,
		uint32_t capacity, void *zrpc_tlv_restrict__ buf)
{
	tlvb->head = 0;
	tlvb->capacity = capacity;
	tlvb->buf = buf;
}


/**
 * @brief Calculate how much space remains in the buffer.
 *
 * @param tlvb The zRPC TLV buffer.
 *
 * @return Number of bytes left in the buffer
 */
inline uint32_t zrpc_tlvb_space(struct zrpc_tlvb const *tlvb)
{
	return tlvb->capacity - tlvb->head;
}


/**
 * @brief Advance the head of the buffer by specified amount.
 *
 * @warning Do not attempt to iterate over a partially skipped buffer unless
 *          you are absolutely certain the skipped portion of the buffer
 *          contains attributes.
 *
 * @param tlvb The TLV buffer.
 * @param len  Number of bytes, relative the current head, to advance the head.
 *
 * @retval 0          Head successfully advanced.
 * @retval -EOVERFLOW CONFIG_ZRPC_PEDANTIC is set and advancing the head by
 *                    @c len bytes would result in a numeric overflow.
 * @retval -ENOBUFS   Attempted advance beyond the end of the buffer.
 */
inline int zrpc_tlvb_skip(struct zrpc_tlvb *tlvb, size_t len)
{
#if CONFIG_ZRPC_PEDANTIC
	if (unlikely(len > UINT32_MAX))
		return -EOVERFLOW;
	if (unlikely(UINT32_MAX - tlvb->head < len))
		return -EOVERFLOW;
#endif
	if (unlikely(tlvb->head + len > tlvb->capacity))
		return -ENOBUFS;

	tlvb->head += len;
	return 0;
}


/**
 * @brief Obtain unqualified pointer to the attribute at the buffer head.
 *
 * Intended primarily for use in zrpc_tlvb_head().
 *
 * @param tlvb Address of the TLV buffer.
 *
 * @return A unqualified poitner to the head of the buffer.
 */
inline struct zrpc_attr *zrpc_tlvb_head_(struct zrpc_tlvb *tlvb)
{
	return (void *)(tlvb->buf + tlvb->head);
}


/**
 * @brief Obtain const-qualified pointer to the attribute at the buffer head.
 *
 * Intended primarily for use in zrpc_tlvb_head().
 *
 * @param tlvb Address of the TLV buffer.
 *
 * @return A const-qualified poitner to the head of the buffer.
 */
inline struct zrpc_attr const *zrpc_tlvb_chead_(struct zrpc_tlvb const *tlvb)
{
	return (void const *)(tlvb->buf + tlvb->head);
}


/**
 * @brief Obtain suitably qualified pointer to head of @c tlvb_.
 *
 * Invokes zrpc_tlvb_head_() or zrpc_tlvb_chead_() depending on the
 * qualifier of the buffer.
 *
 * @param tlvb_ Address of the TLV buffer.
 *
 * @return Address of the <tt>struct zrpc_attr</tt> attribute at
 *	   the buffer head.
 */
#define zrpc_tlvb_head(tlvb_)					\
	_Generic((tlvb_),					\
		struct zrpc_tlvb *: zrpc_tlvb_head_,		\
		struct zrpc_tlvb const *: zrpc_tlvb_chead_	\
	)(tlvb)


/**
 * @brief Get length of populated portion of @c tlvb in bytes.
 *
 * @param tlvb The TLV buffer.
 *
 * @return Number of populated bytes in @c tlvb.
 */
inline size_t zrpc_tlvb_len(struct zrpc_tlvb const *tlvb)
{
	return tlvb->head;
}



/**
 * @brief Obtain non-qualified pointer to the first attribute in @c tlvb.
 *
 * Used primarily by zrpc_tlvb_start().
 *
 * @param tlvb The TLV buffer.
 *
 * @return Address of the first attribute.
 */
inline struct zrpc_attr *zrpc_tlvb_start_(struct zrpc_tlvb *tlvb)
{
	return (void *)tlvb->buf;
}


/**
 * @brief Obtain const-qualified pointer to the first attribute in @c tlvb.
 *
 * Used primarily by zrpc_tlvb_start().
 *
 * @param tlvb The TLV buffer.
 *
 * @return Address of the first attribute.
 */
inline struct zrpc_attr const *zrpc_tlvb_cstart_(struct zrpc_tlvb const *tlvb)
{
	return (void const *)tlvb->buf;
}

/**
 * @brief Optain suitably qualified pointer to the first attribute in @c tlvb_
 *
 * @note Empty TLV buffers yield the same address as tlvb_head(). Use
 *       zrpc_tlvb_empty() to determine whether or not the TLV buffer is
 *	 empty.
 *
 * @param tlvb_ The TLV buffer.
 *
 * @return Suitably qualified pointer to the first attribute.
 */
#define zrpc_tlvb_start(tlvb_)					\
	_Generic((tlvb_),					\
		struct zrpc_tlvb *: zrpc_tlvb_start_,		\
		struct zrpc_tlvb const *: zrpc_tlvb_cstart_	\
	)(tlvb_)


/**
 * @brief Determine whether or not the provided buffer is empty.
 *
 * @param tlvb The TLV buffer.
 *
 * @retval true  The buffer contains at least one attribute.
 * @retval false The buffer contains no attributes.
 */
inline bool zrpc_tlvb_empty(struct zrpc_tlvb const *tlvb)
{
	return zrpc_tlvb_start(tlvb) == zrpc_tlvb_head(tlvb);
}


/**
 * @brief Obtain size of the payload, padding excluded.
 *
 * @sa zrpc_attr_size() for size of the entire attribute save for
 *     the trailing padding and zrpc_attr_total_size() for size of the
 *     entire attribute @b including trailing padding.
 *
 * @param attr Address of the attribute.
 *
 * @return Size of the unpadded payload.
 */
inline uint32_t zrpc_attr_len(struct zrpc_attr const *attr)
{
	return attr->len - sizeof(*attr);
}


/**
 * @brief Obtain size of the attribute.
 *
 * This includes size of the header, the payload and the padding
 * between the two but excludes the potential padding following
 * the payload.
 *
 * @sa zrpc_attr_total_size() for size including the padding at the
 *     end and zrpc_attr_len() for size of only the payload.
 *
 * @param attr Address of the attribute.
 *
 * @return Size of the attribute, padding at the end excluded.
 */
inline uint32_t zrpc_attr_size(struct zrpc_attr const *attr)
{
	return attr->len;
}


/**
 * @brief Calculate size of @c attr, including padding at the end.
 *
 * @sa zrpc_attr_size() for size exlucing padding at the end and
 *     zrpc_attr_len() for size of the payload, padding not included.
 *
 * @param attr Address of the attribute.
 *
 * @return Size of the entire attribute, padding included.
 */
inline uint32_t zrpc_attr_total_size(struct zrpc_attr const *attr)
{
	return zrpc_align(attr->len);
}


/**
 * @brief Obatain unqualified pointer to attribute payload.
 *
 * Intended primarily for used by zrpc_attr_data().
 *
 * @param attr The zRPC attribute.
 *
 * @return A unqualified pointer to the attribute payload
 */
inline uint8_t *zrpc_attr_data_(struct zrpc_attr *attr)
{
	return attr->payload;
}


/**
 * @brief Obatin const-qualified pointer to attribute payload.
 *
 * Intended primarily for used by zrpc_attr_data().
 *
 * @param attr The zRPC attribute.
 *
 * @return A const-qualified pointer to the attribute payload
 */
inline uint8_t const *zrpc_attr_cdata_(struct zrpc_attr const *attr)
{
	return attr->payload;
}


/**
 * @brief Obtain suitably qualified pointer to attribute payload.
 *
 * Invokes either zrpc_attr_data_() or zrpc_attr_cdata_() depending
 * on the qualifier of @c attr.
 *
 * @param attr Pointer to a <tt>struct zrpc_attr</tt>.
 *
 * @return Potentially const-qualified pointer to the payload of @c attr.
 */
#define zrpc_attr_data(attr)					\
	_Generic((attr),					\
		struct zrpc_attr *: zrpc_attr_data_,		\
		struct zrpc_attr const *: zrpc_attr_cdata_	\
	)(attr)


/**
 * @brief Extract 8 bit variable from the payload of @c attr.
 *
 * @note It is the responsibility of the caller ot ensure that
 *       the payload of @c attr actually contains a @c uint8_t
 *       A.instance.
 *
 *       The function validates only the payload size. If this does
 *       not correspond to a @c uint8_t, the value 0 is returned.
 *
 * @param attr The attribute containing the payload.
 *
 * @retval 0-0xff The payload of the attribute.
 * @retval 0      The attribute does not contain a @c uint8_t.
 */
inline uint8_t zrpc_attr_get_u8(struct zrpc_attr const *attr)
{
	if (unlikely(zrpc_attr_len(attr) != sizeof(uint8_t)))
		return 0u;
	return *(uint8_t const *)zrpc_attr_data(attr);
}


/**
 * @brief Extract 16 bit variable from the payload of @c attr.
 *
 * @note It is the responsibility of the caller ot ensure that
 *       the payload of @c attr actually contains a @c uint16_t
 *       A.instance.
 *
 *       The function validates only the payload size. If this does
 *       not correspond to a @c uint16_t, the value 0 is returned.
 *
 * @param attr The attribute containing the payload.
 *
 * @retval 0-0xffff The payload of the attribute.
 * @retval 0        The attribute does not contain a @c uint16_t.
 */
inline uint16_t zrpc_attr_get_u16(struct zrpc_attr const *attr)
{
	if (unlikely(zrpc_attr_len(attr) != sizeof(uint16_t)))
		return 0u;
	return *(uint16_t const *)zrpc_attr_data(attr);
}


/**
 * @brief Extract 32 bit variable from the payload of @c attr.
 *
 * @note It is the responsibility of the caller ot ensure that
 *       the payload of @c attr actually contains a @c uint32_t
 *       A.instance.
 *
 *       The function validates only the payload size. If this does
 *       not correspond to a @c uint32_t, the value 0 is returned.
 *
 * @param attr The attribute containing the payload.
 *
 * @retval 0-0xffffffff The payload of the attribute.
 * @retval 0            The attribute does not contain a @c uint32_t.
 */
inline uint32_t zrpc_attr_get_u32(struct zrpc_attr const *attr)
{
	if (unlikely(zrpc_attr_len(attr) != sizeof(uint32_t)))
		return 0u;
	return *(uint32_t const *)zrpc_attr_data(attr);
}


/**
 * @brief Extract 64 bit variable from the payload of @c attr.
 *
 * @note It is the responsibility of the caller ot ensure that
 *       the payload of @c attr actually contains a @c uint64_t
 *       A.instance.
 *
 *       The function validates only the payload size. If this does
 *       not correspond to a @c uint32_t, the value 0 is returned.
 *
 * @param attr The attribute containing the payload.
 *
 * @retval 0-0xffffffffffffffff The payload of the attribute.
 * @retval 0                    The attribute does not contain a @c uint64_t.
 */
int zrpc_attr_get_u64(struct zrpc_attr const *attr);


/**
 * @brief Extract NUL-terinated string from payload of the provided attribute.
 *
 * @param attr The attribute to extract the string from.
 *
 * @retval !=NULL Address of the extrated string
 * @retval NULL   The attribute is either empty or does not contain a termianted
 *                string.
 */
inline char const *zrpc_attr_get_string(struct zrpc_attr const *attr)
{
	char const *s = zrpc_attr_data(attr);

	if (unlikely(!zrpc_attr_len(attr) || s[zrpc_attr_len(attr) - 1]))
		return NULL;
	return s;
}


/**
 * @brief Obtain unqualified pointer to the attribute following @c attr.
 *
 * Intended primarily for use in zrpc_attr_next().
 *
 * @warning No bounds checking is performed. Callers should verify that the
 *          returned address is valid before dereferencing it.
 *
 * @param attr Address of the current attribute.
 *
 * @return Unqualified pointer to the attribute following @c attr.
 */
inline struct zrpc_attr *zrpc_attr_next_(struct zrpc_attr *attr)
{
	return (void *)((uint8_t *)attr + zrpc_attr_total_size(attr));
}


/**
 * @brief Obtain const-qualified pointer to the attribute following @c attr.
 *
 * Intended primarily for use in zrpc_attr_next().
 *
 * @warning No bounds checking is performed. Callers should verify that the
 *          returned address is valid before dereferencing it.
 *
 * @param attr Address of the current attribute.
 *
 * @return A const-qualified pointer to the attribute following @c attr.
 */
inline struct zrpc_attr const *zrpc_attr_cnext_(struct zrpc_attr const *attr)
{
	return (void const *)((uint8_t const *)attr +
				zrpc_attr_total_size(attr));
}


/**
 * @brief Obtain suitably qualified pointer to the attribute following @c attr.
 *
 * @warning No bounds checking is performed. Callers should verify that the
 *          returned address is valid before dereferencing it.
 *
 * @param attr Pointer to a <tt>struct zrpc_attr</tt>.
 *
 * @return Suitably qualified pointer referring to the attribute following @c
 *         attr.
 */
#define zrpc_attr_next(attr)					\
	_Generic((attr),					\
		struct zrpc_attr *: zrpc_attr_next_,		\
		struct zrpc_attr const *: zrpc_attr_cnext_	\
	)(attr)


/**
 * @brief Insert attribute with arbitrary data into buffer.
 *
 * @param tlvb The TLV buffer.
 * @param tag  Tag identifying the attribute.
 * @param data Address of the data to insert.
 * @param n    Number of bytes to insert.
 *
 * @retval 0       Success.
 * @retval -ERANGE @c CONFIG_ZRPC_PEDANTIC is enabled and @n is large
 *                 enough that it would cause an overflow.
 * @retval -ENOBUFS Not enough space in the buffer.
 */
int zrpc_tlvb_put(struct zrpc_tlvb *zrpc_tlv_restrict__ tlvb, zrpc_tag tag,
		void const *zrpc_tlv_restrict__ data, uint32_t n);


/**
 * @brief Insert 8 bit variable into the TLV buffer.
 *
 * @param tlvb Address of the TLV buffer.
 * @param tag  Tag of the attribute to insert.
 * @param v    Value to include in the attribute
 *
 * @return See zrpc_tlvb_put() return values.
 */
inline int zrpc_tlvb_put_u8(struct zrpc_tlvb *tlvb, zrpc_tag tag, uint8_t v)
{
	return zrpc_tlvb_put(tlvb, tag, &v, sizeof(v));
}


/**
 * @brief Insert 16 bit variable into the TLV buffer.
 *
 * @param tlvb Address of the TLV buffer.
 * @param tag  Tag of the attribute to insert.
 * @param v    Value to include in the attribute
 *
 * @return See zrpc_tlvb_put() return values.
 */
inline int zrpc_tlvb_put_u16(struct zrpc_tlvb *tlvb, zrpc_tag tag, uint16_t v)
{
	return zrpc_tlvb_put(tlvb, tag, &v, sizeof(v));
}


/**
 * @brief Insert 32 bit variable into the TLV buffer.
 *
 * @param tlvb Address of the TLV buffer.
 * @param tag  Tag of the attribute to insert.
 * @param v    Value to include in the attribute
 *
 * @return See zrpc_tlvb_put() return values.
 */
inline int zrpc_tlvb_put_u32(struct zrpc_tlvb *tlvb, zrpc_tag tag, uint32_t v)
{
	return zrpc_tlvb_put(tlvb, tag, &v, sizeof(v));
}


/**
 * @brief Insert 64 bit variable into the TLV buffer.
 *
 * @note Requires @c CONFIG_ZRPC_U64_SUPPORT.
 *
 * @param tlvb Address of the TLV buffer.
 * @param tag  Tag of the attribute to insert.
 * @param v    Value to include in the attribute
 *
 * @return See zrpc_tlvb_put() return values.
 */
inline int zrpc_tlvb_put_u64(struct zrpc_tlvb *tlvb, zrpc_tag tag, uint64_t v)
{
	return zrpc_tlvb_put(tlvb, tag, &v, sizeof(v));
}


/**
 * @brief Insert NUL-terminated string into the TLV buffer.
 *
 * @param tlvb Address of the TLV buffer.
 * @param tag  Tag of the attribute to insert.
 * @param s    The string to insert.
 *
 * @return See zrpc_tlvb_put() return values.
 */
inline int zrpc_tlvb_put_string(struct zrpc_tlvb *tlvb, zrpc_tag tag,
		char const *s)
{
	return zrpc_tlvb_put(tlvb, tag, s, strlen(s) + 1u);
}


/**
 * @brief Iterate over the TLV buffer with @b attr referring to each attribute.
 *
 * @code{.c}
 * struct zrpc_attr const *attr;
 * extern zrpc_tlvb const *tlvb;
 *
 * zrpc_tlvb_foreach(tlvb, attr)
 *	printf("Visited attribute with tag 0x%x\n", (unsigned int)attr->tag);
 * @endcode
 *
 * @param tlvb_ Address of the TLV buffer.
 * @param attr_ A pointer to a <tt>struct zrpc_attr</tt> used to visit each
 *              attribute.
 */
#define zrpc_tlvb_foreach(tlvb_, attr_)						\
	for (struct zrpc_attr const *UTIL_CAT(zrpc_foreach_end, __LINE__) =	\
		(((void)((attr_) = zrpc_tlvb_start(tlvb_))),			\
			zrpc_tlvb_head(tlvb));					\
		(attr_) < UTIL_CAT(zrpc_foreach_end, __LINE__);			\
		(attr_) = zrpc_attr_next(attr_))



/**
 * @brief Iterate over non-const @c tlvb and populate @c attrs.
 *
 * @sa zrpc_tlvb_map().
 *
 * @param tlvb   The TLV buffer.
 * @param attrs  Array of <tt>struct zrpc_attr</tt> attributes to populate.
 * @param nattrs Number of entries in @c attrs.
 *
 * @return Number of entries in @c attrs that were populated.
 */
size_t zrpc_tlvb_map_(struct zrpc_tlvb *tlvb, struct zrpc_attr **attrs,
	size_t nattrs);

/**
 * @brief Iterate over const-qualified @c tlvb and populate @c attrs.
 *
 * @sa zrpc_tlvb_map().
 *
 * @param tlvb   The TLV buffer.
 * @param attrs  Array of <tt>struct zrpc_attr</tt> attributes to populate.
 * @param nattrs Number of entries in @c attrs.
 *
 * @return Number of entries in @c attrs that were populated.
 */
size_t zrpc_tlvb_cmap_(struct zrpc_tlvb const *tlvb,
		struct zrpc_attr const **attrs, size_t nattrs);


/**
 * @brief Iterate over @c tlvb and populate @c attrs.
 *
 * Iterates over the provided TLV buffer and, for each attribute therein, should
 * @c attrs contain a slot at the index matching the tag of said attribute, the
 * slot is set to the address of the attribute. If an attribute occurs multiple
 * times, the entry contains the address of the last encountered attribute.
 *
 * The result is reminiscent of the generic netlink driver-side API provided by
 * Linux.
 *
 * Attributes in @c tlvb may appear in any order. As such, there is no guarantee
 * that e.g. <tt>addrs[0] < attrs[1]</tt> is true even if neither of the two
 * are @c NULL.
 *
 * @code{.c}
 * // Assume the tlvb contains three attribute with tags 0, 1 and 3.
 * extern struct zrpc_tlvb *tlvb;
 * // Look for tags 0, 1, 2 and 3.
 * struct zrpc_attr attrs*[] = {
 *	[3] = NULL,
 * };
 *
 * size_t n = zrpc_tlvb_map(tlvb, attrs, ARRAY_SIZE(attrs));
 * // Should have found attributes 0, 1 and 3.
 * assert(n == 3u);
 * // The first slot contains address of the zrpc_attr with tag 0
 * assert(attrs[0]);
 * // The second slot contains address of the zrpc_attr with tag 1
 * assert(attrs[1]);
 * // The TLV buffer did not contains an attribute with tag 2. Thus,
 * // the third entry is left empty
 * assert(!attrs[2]);
 * // The fourth entry contains the address of the attribute with tag 3
 * assert(attrs[3]);
 *
 * @endcode
 *
 * @param tlvb_  The TLV buffer.
 * @param attrs  Array of <tt>struct zrpc_attr</tt> attributes to populate.
 * @param nattrs Number of entries in @c attrs.
 *
 * @return Number of entries in @c attrs that were populated.
 */
#define zrpc_tlvb_map(tlvb_, attrs, nattrs)					\
	_Generic((tlvb_),							\
		struct zrpc_tlvb *: zrpc_tlvb_map_,				\
		struct zrpc_tlvb const *: zrpc_tlvb_cmap_			\
	)(tlvb_, attrs, nattrs)


/** @} */

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#undef zrpc_tlvb_head
#undef zrpc_tlvb_start

#undef zrpc_attr_data
#undef zrpc_attr_next
#undef zrpc_tlvb_map


inline struct zrpc_attr *zrpc_tlvb_head(struct zrpc_tlvb *tlvb) {
	return zrpc_tlvb_head_(tlvb);
}

inline struct zrpc_attr const *zrpc_tlvb_head(struct zrpc_tlvb const *tlvb) {
	return zrpc_tlvb_chead_(tlvb);
}

inline struct zrpc_attr *zrpc_tlvb_start(struct zrpc_tlvb *tlvb)
{
	return zrpc_tlvb_start_(tlvb);
}

inline struct zrpc_attr const *zrpc_tlvbv_start(struct zrpc_tlvb const *tlvb)
{
	return zrpc_tlvb_cstart_(tlvb);
}

inline uint8_t *zrpc_attr_data(struct zrpc_attr *attr)
{
	return zrpc_attr_data_(attr);
}

inline uint8_t const *zrpc_attr_data(struct zrpc_attr const *attr)
{
	return zrpc_attr_cdata_(attr);
}

inline struct zrpc_attr *zrpc_attr_next(struct zrpc_attr *attr)
{
	return zrpc_attr_next_(attr);
}

inline struct zrpc_attr const *zrpc_attr_next(struct zrpc_attr const *attr) {
	return zrpc_attr_cnext_(attr);
}

inline size_t zrpc_tlvb_map(struct zrpc_tlvb *tlvb, struct zrpc_attr **attrs,
						size_t nattrs)
{
	return zrpc_tlvb_map_(tlvb, attrs, nattrs);
}

inline size_t zrpc_tlvb_map(struct zrpc_tlvb const *tlvb,
		struct zrpc_attr const **attrs, size_t nattrs)
{
	return zrpc_tlvb_cmap_(tlvb, attrs, nattrs);
}

#endif /* __cplusplus */

#endif /* ZEPHYR_INCLUDE_RPC_ZRPC_TLV_H_ */

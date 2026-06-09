/*
 * Copyright (c) 2026 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @ingroup zrpc
 * @brief zRPC core header.
 */

#ifndef ZEPHYR_INCLUDE_RPC_ZRPC_H_
#define ZEPHYR_INCLUDE_RPC_ZRPC_H_

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/sys/util_macro.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zRPC core implementation.
 * @defgroup zrpc zRPC
 * @{
 *
 * @brief zRPC backends
 * @defgroup zrpc_backend zRPC backends
 * @{
 * @}
 */

/** Maximum value of the @c zrpc_tag type */
#define ZRPC_TAG_MAX ((zrpc_tag)~(zrpc_tag)0)

/** Maximum value of the @c zrpc_attrlen type */
#define ZRPC_ATTRLEN_MAX ((zrpc_attrlen)~(zrpc_attrlen)0)

/** Maximum value of the @c zrpc_msglen type */
#define ZRPC_MSGLEN_MAX ((zrpc_msglen)~(zrpc_msglen)0)


/** Tag type */
typedef uint16_t zrpc_tag;


/** Flag type */
typedef uint8_t zrpc_flags;


/** CRC type */
typedef uint8_t zrpc_crc;


/** Attribute length type */
typedef uint16_t zrpc_attrlen;


/** Message length type */
typedef uint32_t zrpc_msglen;


/** zRPC message flags */
enum {
	/** Message is a reply */
	ZRPC_FLAG_REPLY = BIT(0)
};


/** zRPC message attribute */
struct zrpc_attr {
	union {
		struct {
			/** Length, including the header but not the padding */
			zrpc_attrlen len;
			/** Tag */
			zrpc_tag tag;
		};
		/** Unused, for alignment */
		uint32_t align_;
	};
	/** Address of the attribute payload */
	uint8_t payload[];
};


/**zRPC message header */
struct zrpc_msghdr {
	/** RPC identifier */
	zrpc_tag id;
	/** Message flags */
	zrpc_flags flags;
	/** CRC computed over relevant portion of the yaml */
	zrpc_crc crc;
	/** Length of  attribute tail in bytes, size of header @b not included */
	zrpc_msglen len;
	/** Sequence number */
	uint16_t seq;
	/** Trailing attributes */
	struct zrpc_attr attrs[];
};


/**
 * @brief Get next sequence number.
 *
 * @return A 16 bit unique sequence number.
 */
uint16_t zrpc_seq_next(void);


static_assert(alignof(struct zrpc_attr) == alignof(uint32_t),
	"Unexpected attribute alignment");


/** zRPC subsystem API */
__subsystem struct zrpc_driver_api {
	/**
	 * @brief Send message header, including optional trailing attributes.
	 *
	 * @sa zrpc_send().
	 */
	int (*send)(struct device const *dev, struct zrpc_msghdr const *msghdr);

	/**
	 * @brief Receive reply.
	 *
	 * @sa zprc_recv().
	 */
	int (*recv)(struct device const *dev, uint16_t seq,
			struct zrpc_msghdr *msghdr, size_t msg_size);
};


/**
 * @brief Send zRPC message to peer.
 *
 * @param dev    The device associated with the channel.
 * @param msghdr The message to send, already populated.
 *
 * @retval 0      Message successfully sent.
 * @retval -errno An error occurred.
 */
__syscall int zrpc_send(struct device const *dev,
			struct zrpc_msghdr const *msghdr);


static inline int z_impl_zrpc_send(struct device const *dev,
			struct zrpc_msghdr const *msghdr)
{
	struct zrpc_driver_api const *api = dev->api;

	if (unlikely(!api->send))
		return -ENOSYS;

	return api->send(dev, msghdr);
}


/**
 * @brief Receive zRPC reply with given @p seq.
 *
 * The call is allowed to block waiting for a reply and is thus
 * ill-suited to interrupt contexts.
 *
 * @param dev      The device associated with the channel.
 * @param seq      The sequence number of the message to receive.
 * @param msghdr   Buffer to store the reply in.
 * @param msg_size Size of the entire buffer at @p msghdr.
 *
 * @retval 0      Reply received, rpely stored in @p msghdr. Size of the
 *                received message is available in @c msghdr->len.
 * @retval -errno An error occurred.
 */
__syscall int zrpc_recv(struct device const *dev, uint16_t seq,
		struct zrpc_msghdr *msghdr, size_t msg_size);


static inline int z_impl_zrpc_recv(struct device const *dev, uint16_t seq,
		struct zrpc_msghdr *msghdr, size_t msg_size)
{
	struct zrpc_driver_api const *api = dev->api;

	if (unlikely(!api->recv))
		return -ENOSYS;

	return api->recv(dev, seq, msghdr, msg_size);
}

#include <zephyr/syscalls/zrpc.h>

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RPC_ZRPC_H_ */

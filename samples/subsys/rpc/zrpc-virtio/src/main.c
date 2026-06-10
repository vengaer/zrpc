/*
 * Copyright (c) 2026 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/rpc/zrpc-channel-virtio.h>

LOG_MODULE_REGISTER(virtio_sample, CONFIG_ZRPC_VIRTIO_LOG_LEVEL);

/* Invoked when the get_uid RPC is received from the peer */
int zrpc_virtio_get_uid_serve(uint8_t *uid, uint32_t size)
{
	uint32_t r = sys_rand32_get();

	if (size < sizeof(r))
		return -ENOBUFS;

	printf("Sending UID 0x%x\n", (unsigned int)r);
	memcpy(uid, &r, sizeof(r));
	return 0;
}


int zrpc_virtio_forward_rtp_pkt_serve(uint8_t const *rtp_pkt,
		uint32_t rtp_pkt_size, void *user_data)
{
	/*
	 * user_data refers to the string set via
	 * zrpc_virtio_forward_rtp_pkt_set_user_data
	 */
	LOG_INF("User data: %s", (char const *)user_data);
	LOG_HEXDUMP_INF(rtp_pkt, rtp_pkt_size, "RTP packet:");
	return 0;
}


/* Invoked whtn the heartbeat RPC is received from the peer */
int zrpc_virtio_heartbeat_serve(void)
{
	puts("Heartbeat received");
	return 0;
}

int main(void)
{
	int ret;
	uint8_t uid[4u];

	/* Set private data for the forward_rtp_pkt RPC serivcer */
	ret = zrpc_virtio_forward_rtp_pkt_set_user_data("hello");
	if (ret)
		LOG_ERR("Could not set user data for forward_rtp_pkt: %d",
					-ret);

	/* Request UID from peer */
	ret = zrpc_virtio_get_uid(uid, sizeof(uid));
	if (ret) {
		fprintf(stderr, "get_uid RPC failed: %d\n", -ret);
		return ret;
	}

	printf("UID returned by peer: ");
	for (unsigned int i = 0u; i < sizeof(uid); ++i)
		printf("%02x", uid[i]);
	puts("");

	/* Send heartbeat to peer */
	ret = zrpc_virtio_heartbeat();
	if (ret) {
		fprintf(stderr, "Could not send heartbeat: %d\n", -ret);
		return ret;
	}

	return 0;
}

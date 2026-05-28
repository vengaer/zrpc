/*
 * Copyright (c) 2026 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>

#include <zephyr/random/random.h>
#include <zephyr/rpc/zrpc-channel-virtio.h>

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


/* Invoked whtn the heartbeat RPC is received from the peer */
int zrpc_virtio_heartbeat_serve(void)
{
	puts("Heartbeat received");
	return 0;
}

int main(void)
{
	uint8_t uid[4u];
	/* Request UID from peer */
	int ret = zrpc_virtio_get_uid(uid, sizeof(uid));
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

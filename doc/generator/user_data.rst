.. _user_data:

User Data Management
====================

As described in the :ref:`Want User Data <rpc_want_user_data>` section, the
zRPC core may be configured to pass a user-supplied pointer to an RPC servicer.
This pointer is stored as a global variable within the zRPC, meaning that whatever
variable it refers to **must** remain valid for as long as the associated RPC
may be invoked.

Locking Scheme
--------------

As RPC :ref:`servicers <concept_servicer>` and user data assignment may, at least
in theory, execute concurrently, the zRPC core takes care to ensure accesses to said
data being mutually exclusive. This is achieved by using a bucket hashing scheme
where each RPC is associated with one of ``1 << CONFIG_ZRPC_USER_DATA_BUCKET_SHIFT``
buckets, each of which has its own mutex. Whenever the user data for an RPC is being
accessed --- either via the
``zrpc_{{ channel.name | lower }}_{{ rpc.name | lower }}_set_user_data`` function or
from the RPC servicer --- the zRPC core locks the mutex associated with the bucket of
the RPC. Thus, increasing the value of ``CONFIG_ZRPC_USER_DATA_BUCKET_SHIFT`` reduces
contention at the cost of increased memory consumption.

It likely goes without saying but do note that the number of mutexes increases
exponentially with ``CONFIG_ZRPC_USER_DATA_BUCKET_SHIFT``.

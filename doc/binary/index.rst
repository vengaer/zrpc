.. _binary_format:

Binary Format
=============

zRPC encodes RPCs on a binary format reminiscent of
`netlink <https://docs.kernel.org/userspace-api/netlink/intro.html>`__. Each encoded
RPC starts with a fixed-size header immediately followed by attributes encapsulating
parameters and return value.

The zRPC message header looks as follows

.. literalinclude:: /../include/zephyr/rpc/zrpc.h
	:caption: zRPC message header structure.
	:language: c
	:start-at: struct zrpc_msghdr {
	:end-at: };

The ``id`` field holds a numeric identifier uniquely identifying each RPC within a
channel whereas ``flags`` is used to distinguish between requests and replies. The ``crc``
field is computed over parts of the :ref:configuration file <config_yaml>``and used to ensure
that the two :ref:`endpoints <concept_endpoint>` run code compatible with each other. Lastly,
the ``len`` field contains the size of the trailing attributes, in bytes whereas the ``seq``
field is a sequence counter used to match replies against requests.

The ``struct zrpc_attr`` is to zRPC what ``struct nlattr`` is to netlink. It is defined as

.. literalinclude:: /../include/zephyr/rpc/zrpc.h
	:caption: zRPC attribute.
	:language: c
	:start-at: struct zrpc_attr {
	:end-before: /**zRPC message header */

As is evident from the above, each attribute is aligned to a four byte boundary --- just as
``struct nlattr`` instances are. Furthermore, the ``len`` field of each attribute does not
include the size of the potential padding following it in an encoded RPC.

The zRPC core constructs and traverses attribute sequences using the ``stuct zrpc_tlvb`` buffer
abstraction.

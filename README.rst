zRPC --- A Generic RPC Framework for Zephyr
===========================================

zRPC endeavors to bring bidirectional, transport-agnostic
`remote procedure calls <https://en.wikipedia.org/wiki/Remote_procedure_call>`__
to Zephyr. The module is built on top of a simple driver API, allowing RPCs to be
issued across arbitrary transport mediums such as shared memory or Ethernet.

At the time of writing, the only medium supported by the module is shared memory
via the OpenAMP-based VirtIO backend.

Using the Module
----------------

Firstly, add zRPC to your west manifest as one normally would.

.. code-block:: yaml
        :caption: West manifest example.

        manifest:
          remotes:
            - name: vengaer
              url-base: https://github.com/vengaer
          projects:
            remote: vengaer
            revision: <preferred-rev>

Enable ``CONFIG_ZRPC``, select an appropriate zRPC backend and set
``CONFIG_ZRPC_CONFIGURATION`` s.t. it refers to a configuration file ---
see :ref:`At a Glance <at_a_glance>` for a brief description of the
configuration format. The path to the file may be either absolute or
relative the west top directory.

Lastly, implement the required ``zrpc_{{ channel.name }}_{{ rpc.name }}_serve``
function(s) --- again, refer to :ref:`At a Glance <at_a_glance>` --- and build
Zephyr as one normally would.

.. _at_a_glance:

At a Glance
-----------

One of the more central parts of the zRPC module is the configuration file. This
is a yaml file in which RPCs are specified. The latter are split across channels,
each of which may support different RPCs.

.. code-block:: yaml
        :caption: A configuration sample

        channels:
          - name: virtio
            id: 0
            rpcs:
              - name: heartbeat
                parameters:
                  - name: seq
                    type: uint32_t
                    description: |
                      Heartbeat sequence number.
                brief: Send heartbeat to peer.
                description: |
                  Send a heartbeat to the peer. Invoking this periodically with
                  a monotonically increasing sequence number @p seq enables the
                  peer detect whenever this endpoint is reset.
                origin: [ host, remote ]

The above snippet defines an RPC channel supporting a single RPC named
``heartbeat`` which takes a single ``uint32_t`` parameter. The RPC origin
is set to both ``host`` and ``remote``, meaning that the RPC may be issued by
either endpoint. While building the module, the above is translated to
an ``extern`` function along the lines of.

.. code-block:: c
        :caption: The generated heartbeat RPC signature

        /**
         * @brief Send heartbeat to peer
         *
         * Send a heartbeat to the peer. Invoking thisperiodically with
         * a monotonically increasing sequence number @p seq enables the
         * peer to detect whenever this endpoint is reset.
         *
         * @param[in] seq Heartbeat sequence number.
         *
         * @retval >=0    Success.
         * @retval -errno An error occurred.
         */
        int zrpc_virtio_heartbeat(uint32_t seq);

The name is simply the result of concatenating the ``zprc`` prefix, the
name of the channel and the name of the RPC, each part separated by an
underscore.

The module expects the receiving endpoint(s) to implement the function
``{{ rpc.name }}_serve`` which, apart from the name, has the exact same
signature as the RPC. For the heartbeat example, one might write something
akin to

.. code-block:: c
        :caption: Heartbeat servicer

        int zrpc_virito_heartbeat_serve(uint32_t seq)
        {
                LOG_INF("Heartbeat with seq 0x%" PRIx32 " received from peer",
                        seq);
                return 0;
        }

Executing ``zrpc_virtio_heartbeat`` on one endpoint results in
``zrpc_virtio_heartbeat_serve`` being invoked by the other.

.. _overview:

Overview
========


.. _reporting_errors:

Reporting Errors Back to Peer
-----------------------------

Naturally, the :ref:`endpoint <concept_endpoint>` in which an RPC originates will
likely want to know whether or not an error occurred when its peer processed the
RPC. zRPC allows sharing such information seamlessly --- simply return a negative
``errno`` value from the RPC :ref:`servicer <concept_servicer>` and this will be
returned from the RPC call in the origin endpoint.

Consider the following RPC servicer

.. code-block:: c
	:caption: RPC servicer queueing up fixed-sized memory blocks in the receiving endpoint.

	K_MSGQ_ALIGN(msgq, sizeof(uint8_t const *), 8, alignof(uint8_t const *));

	int zrpc_ch0_put_data_service(uint8_t const *data)
	{
		return k_msgq_put(&msgq, data, K_NO_WAIT);
	}

If the :ref:`origin <concept_origin>` invokes the ``zrpc_ch0_put_data`` RPC at a rate higher than
that with which messages are extracted from``msgq``, the latter will eventually become full at which
point the call to ``k_msgq_put`` will fail with ``-ENOMSG``. In such an even, this return value is
embedded in the RPC response and returned from the ``zrpc_ch0_put_data`` function invoked in the
origin.

.. note::

	This behavior causes the origin to block until the receiving endpoint has finished processing
	the RPC and the reply has found its way back to the origin endpoint. For certain applications,
	such delays may be unacceptable and/or unnecessary. For such use cases, the zRPC core can be
	configured to omit replies for specified RPCs using the :ref:`want_reply <rpc_want_reply>` RPC
	property.

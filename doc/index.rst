zRPC
====

zRPC is a Zephyr module implementing generic, serverless
`remote procedure calls <https://en.wikipedia.org/wiki/Remote_procedure_call>`__. In stark
contrast to more well-known RPC implementations such as `pw_rpc <https://pigweed.dev/pw_rpc/>`__
and `Apache Thrift <https://thrift.apache.org/>`__, zRPC is designed specifically for Zephyr
and is as such capable of running with a significantly smaller memory footprint.

The module effectively operates on two levels --- the first being the :ref:`RPC generator <rpc_generation>`,
the second the :ref:`RPC transport medium <backends>`.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   overview
   concepts
   configuration/index
   backends/index
   binary/index
   building

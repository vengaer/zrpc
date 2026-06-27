.. _backends:

Backends
========

zRPC backends implement means of transporting RPCs between channel endpoints via different
mediums. These mediums could, in theory, be almost anything that allows data transfer.

At the time of writing, the only backend provided by zRPC leverages shared memory.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   virtio

.. _generator:

Code Generation
===============

Naturally, RPC generation is a central part of zRPC. This generation is achieved using
`jinja <https://jinja.palletsprojects.com/en/stable/>`__ templates found under
``scripts/zrpc/templates``.

As described :ref:`here <synthesis_filename>`, each RPC channel defined in the
:ref:`YAML configuration file <config_yaml>` corresponds to a distinct header. Additionally,
the module generates an always-present header included by the zRPC core.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   synthesis
   user_data

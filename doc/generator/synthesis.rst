.. _synthesis:

Name Synthesis
==============

Using zRPC requires an understanding of how the names of the API functions
and the headers generated for each channel are synthesized. While the
schema is trivial, expressly documenting should nevertheless hopefully spare
someone the annoyance of having to figure it out for themselves.

RPC and Servicer Names
----------------------

Names synthesized by zRPC combine the name of the :
ref:`channel <concept_channel>` and the name of the RPC itself. These are both
passed through the ``lower`` filter during synthesis. The jinja2 expression
look as follows

.. code-block:: jinja
	:caption: RPC name synthesis expression

	zrpc_{{ channel.name | lower }}_{{ rpc.name | lower }}

The name of the :ref:`servicer <concept_servicer>` is simply the above with the
``_service`` suffix, i.e.

.. code-block:: jinja
	:caption: RPC servicer name synthesis expression

	zrpc_{{ channel.name | lower }}_{{ rpc.name | lower }}_service

The above means that even should the configuration file use uppercase letters
as part of the channel or RPC name, the generated code still uses all lowercase.

.. _synthesis_filename:

Filenames
---------

Accessing function signature of an RPC or a servicer declared by zRPC requires that
the header generated for the channel in which the RPC is defined by included. For a
channel :ref:`named <config_channel_name>` ch0, this is done as follows

.. code-block:: c
	:caption: Including the zRPC header for channel ``ch0``

	#include <zephyr/rpc/zrpc-channel-ch0.h>

The name of the header is synthesized using the following jinja2 expression

.. code-block:: jinja
	:caption: RPC channel header synthesis expression

	zrpc-channel-{{ channel.name | lower }}.h

All generated headers are located such that they may be included using the relative
path ``zephyr/rpc/zrpc-channel-{{ channel.name | lower }}.h``.


User Data Modifier
------------------

As noted in the :ref:`Want User Data <rpc_want_user_data>` section, the zRPC core may be
configured to associate a generic pointer with an RPC handler. If the section describing
an RPC in the configuration file has ``want_user_data`` set to ``True``, the generation
scripts emits a function synthesized using

.. code-block:: jinja
	:caption: RPC user data associator synthesis expression

	zrpc_{{ channel.name | lower }}_{{ rpc.name | lower }}_set_user_data

Said function takes a single ``void *`` pointer and its signature emitted in the
header of the channel containing the RPC. Refer to the :ref:`Filenames <synthesis_filename>`
section for more information on the format of name of the header.

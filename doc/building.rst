.. _building:

Building
========

zRPC is built using west by simply including it in the west manifest of the manifest
repository.

.. code-block:: yaml
        :caption: West manifest example.

        manifest:
          remotes:
            - name: vengaer
              url-base: https://github.com/vengaer
	    # ...

          projects:
            remote: vengaer
            revision: <preferred-rev>
	    #...

With the above in place, run ``west update`` followed by either ``menuconfig`` or
``guiconfig`` and you should see a new module named ``zrpc``. Here, you need to enable
``CONFIG_ZRPC`` along with a :ref:`backend <concept_backend>` suitable for your needs.
Additionally, the ``CONFIG_ZRPC_CONFIGURATION`` option should be set to refer to the
:ref:`YAML configuration file <config_yaml>`.

Having enabled the necessary configuration options, the appropriate device nodes must
now be configured. Exactly how this is done depends on the backend in question. Refer to
the :ref:`backends <backends>` part of the documentation for more information about
your backend of choice.

Dependencies
------------

Building zRPC requires a few tools --- specifically python packages --- not used by upstream
Zephyr. These are easily installed from the ``requirements.txt`` file in the root of the
repository.

.. code-block:: bash
	:caption: Installing requirements

	python3 -m pip install -r requirements.txt

.. note::

	Using pip on a modern-ish Linux distribution will likely force you to set up a
	`venv <https://docs.python.org/3/library/venv.html>`__. That said, since you are presumably
	already building Zephyr, you have almost assuredly solve the externally managed annoyance
	at the point where you may be considering zRPC.

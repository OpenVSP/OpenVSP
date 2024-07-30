.. role:: py(code)
   :language: python

openvsp_config Documentation
============================
The OpenVSP python API can be loaded in different modes.  Since it is not possible to pass options directly to a python
package during import, :py:`openvsp_config` is a configuration module that holds variables that the :py:`openvsp` package
uses to set how it is loaded.

At present, the :py:`openvsp_config` has two attributes that control how :py:`openvsp` is loaded, :py:`LOAD_GRAPHICS` and
:py:`LOAD_FACADE`, which can be set to :py:`True` or :py:`False`.  Although these options control how the OpenVSP API is loaded,
these differences should otherwise be transparent to the user.  I.e. no changes to a user's code are required when
changing from one API mode to another.

No matter how you want to use the OpenVSP API, you should import :py:`openvsp_config`, set the desired attribute options,
and then import :py:`openvsp` itself.


'Normal' OpenVSP API
--------------------

As illustrated here, conventional users of the OpenVSP API will sett both options to :py:`False` before loading :py:`openvsp`.

.. code-block:: python

    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = False
    openvsp_config.LOAD_FACADE = False
    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )

Note that the default for both options is :py:`False`, so the above code block is equvilent to the next code block where
:py:`openvsp_config` was imported before importing :py:`openvsp`, without setting any attribute values in-between.

..  code-block:: python

    import openvsp_config
    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )

In fact, if you do not import :py:`openvsp_config` at all like in the next example, the default values will be applied and
you will still get the default behavior.  This behavior is retained to provide backwards compatibility to existing users
of the OpenVSP API.

..  code-block:: python

    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )

If you are a conventional user of the OpenVSP API, you may be wondering why you should bother importing
:py:`openvsp_config` and setting the option attributes at all.  Importing :py:`openvsp_config` and setting the option
attributes (even when they are not strictly needed) documents your intent and makes your use of :py:`openvsp` most clear.

OpenVSP API with GUI
--------------------

The OpenVSP API can be used with a complete working version of the OpenVSP GUI.  To load the GUI version of :py:`openvsp`,
set :py:`openvsp_config.LOAD_GRAPHICS = True` before you import :py:`openvsp` as shown below.

..  code-block:: python

    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = False
    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )
    vsp.InitGUI()
    vsp.StartGUI()

Note that when the GUI is running in this mode, complete program flow control has been transferred to the GUI.  While
the GUI is fully interactive, Python execution will halt until the user stops the GUI by selecting :py:`Stop GUI` from the File
menu in the OpenVSP GUI.

OpenVSP API in Separate Python Process
--------------------------------------

The OpenVSP API can also be loaded into a separate Python process.  In this case, communication between processes
is handled transparently by a local facade that communicates requests to a server on the other process via sockets.
To load this version, set :py:`openvsp_config.LOAD_FACADE = True` before you import :py:`openvsp` as shown below.

..  code-block:: python

    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = False
    openvsp_config.LOAD_FACADE = True
    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )


OpenVSP API with GUI in Separate Python Process
-----------------------------------------------

These options can be combined, which will load the OpenVSP API with a GUI in a separate Python process.  In this case
the OpenVSP GUI can be fully interactive while also returning program flow control to Python.  This allows OpenVSP GUI
to be active concurrent with an ongoing Python program using the same model in memory.  This mode of operation is
depicted below.

..  code-block:: python

    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    import openvsp as vsp

    vsp.ReadVSPFile( "example.vsp3" )
    vsp.InitGUI()
    vsp.StartGUI()

An astute reader may wonder why a separate process was required to achieve a non-blocking GUI.  Perhaps a more
straightforward approach of running the OpenVSP GUI in a separate thread could have sufficed.  Indeed, a threading
approach would have worked for many cases.  However, a separate process is required to allow the primary Python
thread to retain the capability to execute a GUI of its own.

Loading OpenVSP with both the :py:`LOAD_GRAPHICS` and :py:`LOAD_FACADE` options allows us to develop a Python GUI application
that uses the OpenVSP API and that can have a fully interactive OpenVSP GUI operating concurrent to the application's
own GUI.

The :py:`LOAD_FACADE` option will be further developed to enable other powerful use cases.  Stay tuned.


Contents
=========

.. toctree::
   :maxdepth: 2

   openvsp

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


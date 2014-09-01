
About MT7
=========

MT7 is the evolution of MT5 format.
It is quite similar to it, here are some differences:

    * faces are written directly with no compression
    * texture coordinates are in the float section


Installing into blender
=======================

Copy this whole directory to:

    $blender_directory/scripts/addons/

e.g:

    .config/blender/2.69/scripts/addons/

Then press ``F8``


Running as a standalone script
==============================

You can run the script, with MT7 environment variable

    $ export MT7=$f 
    $ blender --background --python ./mt7\_loader.py

This will generate a snapshot.

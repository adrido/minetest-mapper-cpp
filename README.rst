Minetest Mapper
###############

A tool to generate maps of minetest and freeminer worlds.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Quick links (functional on github. Elsewhere possibly not):

:All features:		`<doc/features.rst>`_
:User manual:		`<doc/manual.rst>`_
:Changelog:		`<Changelog>`_
:Building:		`<doc/build-instructions.rst>`_
:Github:		`<https://github.com/Rogier-5/minetest-mapper-cpp>`_
:Bugs:			`<https://github.com/Rogier-5/minetest-mapper-cpp/issues>`_

Features (short list)
=====================

Map Generation Features
-----------------------

* Support for both minetest and freeminer worlds
* Support for sqlite3, leveldb and redis map databases
* Generate a subsection of the map, or a full map
  (but note that the size of generated images is limited)
* Generate regular maps or height-maps
* All colors for regular or height maps are configurable
* Draw player positions
* Draw different geometric figures, or text on the map
* Draw the map at a reduced scale. E.g. 1:4.
* Draw a scale on the left and/or top side of the map,
  and/or a height scale (for height maps) on the bottom.
* Optionally draw some nodes transparently (e.g. water)
* User Manual

Build Features
--------------
* Supports both the gcc and clang compiler suites
* Build windows, rpm, deb and/or tar.gz installation
  packages. Or simply type 'make install'.

Quick Start
===========

Install Dependencies
--------------------

Make sure the following packages are installed:

**Libraries:**

* libgd
* sqlite3
* leveldb (if leveldb support is desired)
* hiredis (if redis support is desired)

**Build environment:**

* C++ compiler suite (clang or gcc)
* cmake
* make


Compile
-------

Plain:

::

    cmake .
    make

With levelDB and Redis support:

::

    cmake -DENABLE_LEVELDB=true -DENABLE_REDIS=true .
    make

Generate a Map
----------------

After compilation, and optional installation, minetestmapper is started as follows:

('\\' is a continuation character - the command-line continues
on the next line. Type return only after the line that does
not end with the continuation character)


::

    minetestmapper \
	--input <world-directory> \
	--output <image-file-name.png>

If the world is not too large, and if minetestmapper is installed in
a  system directory, it will most likely work as expected.

Possibly, minetestmapper will not be able to find a colors.txt file. If that happens,
the colors.txt file can be specified on the command-line:

::

    minetestmapper \
	--input <world-directory> \
	--output <image-file-name.png> \
	--colors <filename>

Or the colors.txt file must be installed in a location where minetestmapper will find it.
A colors.txt file (named ``colors.txt``, in lowercase) in the world's directory will certainly
be found. Depending on the system and the configuration, other locations are available. Use the
following command to find out which:

::

    minetestmapper \
	--input <world-directory> \
	--output <image-file-name.png> \
	--verbose-search-colors=2

Manual
======

For more elaborate compilation and installation documentation, please consult
the build manual: `<doc/build-instructions.rst>`_.

For more elaborate usage instruction, please consult the user manual: `<doc/manual.rst>`_.

What is this?
=============

This is a map editor for OpenTibia, which is an open source implementation of the MMORPG Tibia (which can be found at [http://tibia.net tibia.net].
You can find project for running your own server at [https://github.com/opentibia/server the otserv project]. The main fansite for instruction and help is [http://otland.net otland.net].

I want to contribute
====================

Contributions are very welcome, if you would like to make any changes, fork this project or request commit access.
I’m liberal to allowing any and all help and my involvement will be restricted to reviewing changes for now.
Please, if you would like to contribute anything,  documentation, extensions or code speak up!

Installation instructions
=========================

Windows
=======
If you have an up-to-date version of windows, you should be able to run the program right off the bat.

If you don't have them already, you will need the following dll files:
*iconv.dll
*libxml2.dll
*msvcp100.dll (SP1)
*msvcr100.dll (SP1)
*zlib1.dll
*archive.dll

Download them (google the filenames) and put them in the same folder as the RME.exe file.

If the mapeditor still won't load, you will need to install the NET 2.0 framework (only relevant for Windows XP).

Download this from: http://www.microsoft.com/downloads/details.aspx?familyid=0856eacb-4362-4b0d-8edd-aab15c5e04f5&displaylang=en

Linux
===============

Assuming that you have all required tools to compile (gcc, cmake, the libraries, etc) clone the project, open you terminal and do the following commands:

```
mkdir build && cd build
cmake ..
make -j4
ln -s build/rme rme
./rme
```

If any library is missing then you need to find it and install in your linux by yourself, required libraries are:

* wxWidgets 3.0
* LibXML2
* Boost
* LibArchive

What is this?
=============

This is a map editor for OpenTibia, which is an open source implementation of the MMORPG Tibia (which can be found at [tibia.com](http://tibia.com)), the official website is [remeresmapeditor.com](http://remeresmapeditor.com).
You can find the project for hosting your own server at [the otserv project](https://github.com/opentibia/server).
The main fansite for help, discussion and servers is [otland.net](http://otland.net).

I want to contribute
====================

Contributions are very welcome, if you would like to make any changes, fork this project or request commit access.
I (Remere) am liberal to allowing any and all help and my involvement will be restricted to reviewing changes for now.
Please, if you would like to contribute anything, documentation, extensions or code speak up!

Bugs
======

Have found a bug? Please create an issue in our [bug tracker](https://github.com/hjnilsson/rme/issues)

Other Applications
==========

* To host your MMORPG game server, you can use [The Forgotten Server](https://github.com/otland/forgottenserver).
* To play your MMORPG game, you can use [OTClient](https://github.com/edubart/otclient)
* To map your MMORPG game, you can use this map editor.

Download
========

You can find official releases at [remeres mapeditor website](http://remeresmapeditor.com/marklar.php).

If you looking for the 3.0 version, download it [here](https://github.com/hjnilsson/rme/releases/) until It is added to the official website.

Compiling
==========

First make sure that you have the necessary compiling, if any library is missing then you need to find it and install in your linux by yourself, required libraries are:

* wxWidgets >= 3.0
* Boost >= 1.55.0

### Windows
==============

Assuming that you have MSVC 2013 or later and the required libraries installed, clone the project, open the project file in `vcproj/Project/RME.vcproj` then hit build.

For more details you can follow [this tutorial](http://otland.net/threads/compiling-remeres-map-editor-from-the-latest-source-with-msvc-2013.216826/) by dominique120.

### Linux
===============

```bash

# Installing dependencies
sudo apt-get install git cmake libboost-system-dev libboost-thread-dev libglu1-mesa-dev libwxgtk3.0-dev libarchive-dev freeglut3-dev libxmu-dev libxi-dev

# Clone the project
git clone https://github.com/hjnilsson/rme.git

# Go to RME Folder
cd rme

# Preparing to build
mkdir build && cd build
cmake ..

# Building
make -j `nproc`

# Running and enjoy
./rme
```

### macOS
===============
Install the package manager Homebrew: http://brew.sh
```bash

# Install dependencies
brew install git cmake wxmac boost libarchive

# Clone the project
git clone https://github.com/hjnilsson/rme.git

# Create a build directory and build
mkdir rme/build && cd rme/build && cmake .. && make

# Run
./rme

```

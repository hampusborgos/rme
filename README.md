What is this?
=============

This is a map editor for OpenTibia, which is an open source implementation of the MMORPG T ibia (which can be found at [http://tibia.net tibia.net]. You can find project for running your own server at [https://github.com/opentibia/server the otserv project]. The main fansite for instruction and help is [http://otland.net otland.net].

I want to contribute
====================

Contributions are very welome, if you would like to make any changes, fork this project or request commit access. I’m liberal to allowing any and all help and my involvement will be restricted to reviewing changes for now. Please, if you would like to contribute anything,  documentation, extensions or code speak up!

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
	
LINUX (Ubuntu)
===============

easy-way: Click the rme-install file, make sure to run it with administrative privileges.

step-by-step guide:

NOTE: you will to be connected to the internet. Open a terminal window.
	
Run the following commands in the terminal:

`sudo apt-get curl`

`curl http://www.tt-solutions.com/vz/key.asc | apt-key add -`

If the above doesn't work, try:

`wget http://www.tt-solutions.com/vz/key.asc -O - | sudo apt-key add -`
	
Then do (note that upgrading might mess up dependencies for other things running on your system. If you want to be careful try without running update first):

`sudo apt-get upgrade`

`sudo apt-get install libwxgtk2.9-0 libboost1.50-all-dev libxml2 libglu1-mesa libgl1-mesa-glx`

Now extract the files from the mapeditors archive file. If you cannot open it, please download p7zip:

`sudo aptitude install p7zip`

Then rightclick on the file called RME, go to properties, Rights and check the "Allow execution as a program" checkbox.

OTHER LINUX DISTROS:
====================
You need libwxgtk2.11.0 or later, and opengl runtime libraries, you may have to compile this yourself depending on whether it's available on your distribution from the start, you do NOT want the unicode version of these files.
	
	(If you have a guide to submit please contact me on github)
	

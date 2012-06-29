WINDOWS:
	If you have an up-to-date version of windows, you should be able to run the program right off the bat.
	
	If you don't have them already, you will need the following dll files:
	iconv.dll
	libxml2.dll
	MSVCP80.dll
	msvcr80.dll
	zlib1.dll
	
	Download them (google the filenames) and put them in the same folder as the RME.exe file.
	If the mapeditor still won't load, you will need to install the NET 2.0 framework.
	Download this from: http://www.microsoft.com/downloads/details.aspx?familyid=0856eacb-4362-4b0d-8edd-aab15c5e04f5&displaylang=en
	
LINUX (Ubuntu):

easy-way:
	Click the rme-install file, make sure to run it with administrative privileges.

step-by-step guide:
	NOTE: you will to be connected to the internet.
	Open a terminal window.
	
	Type: sudo gedit /etc/apt/sources.list
	Add the following line at the end:
		deb http://apt.tt-solutions.com/ubuntu/ dapper main

	Run the following commands in the terminal:
	sudo apt-get curl
	curl http://www.tt-solutions.com/vz/key.asc | apt-key add -
	If the above doesn't work, try:
	wget http://www.tt-solutions.com/vz/key.asc -O - | sudo apt-key add -
	
	Then do:
	sudo apt-get upgrade
	sudo apt-get install libwxgtk2.8-0 libboost1.40-all-dev libxml2 libglu1-mesa libgl1-mesa-glx

	Now extract the files from the mapeditors archive file. If you cannot open it, please download p7zip:
	sudo aptitude install p7zip

	Then rightclick on the file called RME, go to properties, Rights and check the "Allow execution as a program" checkbox.

OTHER LINUX DISTROS:
	You need libwxgtk2.8.0, and opengl runtime libraries, you may have to compile this yourself depending on whether it's available
	on your distribution from the start, you do NOT want the unicode version of these files.
	
	(If you have a guide to submit please contact me, Remere)
	

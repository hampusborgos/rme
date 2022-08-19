#!/usr/bin/python
"""
Converts all .png files into this directory to a .h & one .cpp
file.
"""
import sys
import os
import os.path
import re
import array
import glob

r = re.compile("^([a-zA-Z._][a-zA-Z._0-9]*)[.][pP][nN][gG]$")

header_file = open("pngfiles.h", "wb")
source_file = open("pngfiles.cpp", "wb")

header_file.write("#ifndef __PNG_HEADER_FILE_H__\n")
header_file.write("#define __PNG_HEADER_FILE_H__\n")

png_files = glob.glob("*.png")

for path in png_files:
	filename = os.path.basename(path)
	m = r.match(filename)
	# Allow only filenames that make sense
	# as C variable names
	if not(m):
		print "Skipped file (unsuitable filename): " + filename
		continue

	# Read PNG file as character array
	bytes = array.array('B', open(path, "rb").read())
	count = len(bytes)

	# Output to header
	header_file.write("extern unsigned char " + m.group(1) + "_png[" + str(count) + "];\n")
	# Create the C file
	text = "/* " + filename + " - " + str(count) + " bytes */\nunsigned char " + m.group(1) + "_png[" + str(count) + "] = {\n"

	# Iterate the characters, we want
	# lines like:
	#   0x01, 0x02, .... (8 values per line maximum)
	i = 0
	count = len(bytes)
	for byte in bytes:
		# Every new line starts with two whitespaces
		if (i % 8) == 0:
			text += "  "
		# Then the hex data (up to 8 values per line)
		text += "0x%02x" % (byte)
		# Separate all but the last values
		if (i + 1) < count:
			text += ", "
		if (i % 8) == 7:
			text += '\n'
		i += 1

	# Now conclude the C source
	text += "};\n/* End Of File */\n"

	source_file.write(text)

header_file.write("#endif //__PNG_HEADER_FILE_H__\n")


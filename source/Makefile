CXXFLAGS=`wx-config --version=2.8 --cppflags` -I/usr/include/libxml2/ -g -c
LIBS=-lxml2 -lGL `wx-config --version=2.8 --cppflags` `wx-config --libs` `wx-config --version=2.8 --libs gl` -lboost_thread-mt

objects = main.o pngfiles.o

all: rme data

main.o : main.cpp *.h *.hpp 
	g++ $(CXXFLAGS) -o main.o main.cpp

pngfiles.o : pngfiles.cpp
	g++ $(CXXFLAGS) -o pngfiles.o pngfiles.cpp

json_spirit_reader.o: json/*.h json/json_spirit_reader.cpp
	g++ $(CXXFLAGS) -o json_spirit_reader.o json/json_spirit_reader.cpp
json_spirit_value.o: json/*.h json/json_spirit_value.cpp
	g++ $(CXXFLAGS) -o json_spirit_value.o json/json_spirit_value.cpp
json_spirit_writer.o: json/*.h json/json_spirit_writer.cpp
	g++ $(CXXFLAGS) -o json_spirit_writer.o json/json_spirit_writer.cpp
	 

rme: main.o pngfiles.o json_spirit_writer.o json_spirit_value.o json_spirit_reader.o
	g++ -g $(LIBS) json_spirit_value.o json_spirit_reader.o json_spirit_writer.o main.o pngfiles.o -o rme

release: main.o pngfiles.o
	g++ -O2 $(LIBS) main.o pngfiles.o -o rme-release
	strip --strip-all rme-release

data:
	ln -s ../data/ ./
	ln -s ../extensions/ ./
	ln -s ../COPYING.txt COPYING.txt
	ln -s ../README.txt README.txt
	echo "Note, this Makefile only generates symbolic links to data/, extensions/, COPYING.txt and README.txt."

clean:
	rm rme rme_release *.o COPYING.txt README.txt data extensions


CROSS_COMPILE=arm-linux-

CXX	  		=$(CROSS_COMPILE)g++
CC	  		=$(CROSS_COMPILE)gcc
AR	  		=$(CROSS_COMPILE)ar
STRIP 		=$(CROSS_COMPILE)strip

CXXFLAGS 	= -std=c++11 -Wall -I. -I./mp4v2
LDLIBS 		= -lpthread libmp4v2.a

all: main

main: main.o mp4v2_mp4.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

mp4v2_mp4.o: mp4v2_mp4.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: all
	sudo LD_LIBRARY_PATH=/usr/local/lib ./main

copy: all
	sudo cp main video.h264 audio.aac $(HOME)/FTEL/nFS/MP4v2Test

clean:
	rm -f main main.o mp4v2_mp4.o

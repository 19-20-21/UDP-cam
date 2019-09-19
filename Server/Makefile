.PHONY: all clean

all: run

run:
	cd build && mkdir video && mkdir img
	g++ src/Server.cpp src/UDPSock.cpp `pkg-config --cflags --libs opencv` -lpthread -o build/Server
	chown -R curled:curled ./*

clean:
	rm -rf build/*

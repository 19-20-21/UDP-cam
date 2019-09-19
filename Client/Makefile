.PHONY: all clean

all: run

run:
	g++ src/Client.cpp src/UDPSock.cpp `pkg-config --cflags --libs opencv` -lpthread -o build/Client
	chown -R curled:curled ./*

clean:
	rm -rf build/*

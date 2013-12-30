LIBS=xcb xcb-randr
CXXFLAGS=-std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

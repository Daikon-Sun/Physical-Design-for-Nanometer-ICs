#!/bin/bash
OUT:=$(patsubst %.cpp, %.out, $(wildcard *.cpp))
CFLAGS:=-O3
GCCVERSION:=$(subst .,0, $(shell gcc -dumpversion))
old:=$(shell echo $(GCCVERSION)\<50200 | bc)
ifeq "$(old)" "1"
	CFLAGS+=-std=c++1y
else
	CFLAGS+=-std=c++14
endif

all: $(OUT)

%.out: %.cpp
	g++	$(CFLAGS) $< -o $*

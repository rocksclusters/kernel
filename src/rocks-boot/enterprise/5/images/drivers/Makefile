##
## A sample Makefile for building driver disks
##

KVERSIONS := $(shell \
		if [ -f kversions ]; \
		then \
			cat kversions | sed 's/\([^\#]*\)\#.*/\1/' ; \
		else \
			uname -r ; \
		fi)
KARCH := $(shell uname -m)

SUBDIRS := $(shell \
		if [ -f subdirs ]; \
		then \
			cat subdirs | sed 's/\([^\#]*\)\#.*/\1/' ; \
		fi)

FSTYPE=ext2

SU=/bin/su

TOPDIR := $(shell /bin/pwd)

.PHONY: all

all: sources diskiso
#all: sources diskimg diskiso

include Build.rules

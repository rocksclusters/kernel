#
# $Id: Makefile,v 1.2 2009/09/02 21:11:45 bruno Exp $
#
# @COPYRIGHT@
# @COPYRIGHT@
#
# $Log: Makefile,v $
# Revision 1.2  2009/09/02 21:11:45  bruno
# save the file locally. if it exists, then don't ask for it over the
# network, just stream the file from the local disk
#
# Revision 1.1  2009/08/28 21:49:53  bruno
# the start of the "most scalable installer in the universe!"
#
#

REDHAT.ROOT     = $(CURDIR)/../../

-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

INCLUDE	= -I/usr/include/apr-1
#LIBS	= -L/opt/rocks/lib -lcurl
LIBS	= -lcurl
#EXTRA	= -DDEBUG

build:	tracker-client.cgi

tracker-client.cgi:	tracker-client.c
	cc $(INCLUDE) $(EXTRA) -o tracker-client.cgi tracker-client.c $(LIBS)


install::
	mkdir -p $(ROOT)/tracker

	cp tracker-client.cgi $(ROOT)/tracker

clean::
	rm -f $(NAME).spec.in
	rm -f tracker-client.cgi


#
# $Id: Makefile,v 1.2 2010/02/25 05:45:48 bruno Exp $
#
# @COPYRIGHT@
# @COPYRIGHT@
#
# $Log: Makefile,v $
# Revision 1.2  2010/02/25 05:45:48  bruno
# makin' progress
#
# Revision 1.1  2010/02/24 20:27:32  bruno
# moved new tracker from base roll to kernel roll
#
# Revision 1.4  2009/09/25 21:02:04  bruno
# got prediction code in
#
# Revision 1.3  2009/09/17 20:12:49  bruno
# lots of good stuff:
#  - expandable, circular hash table
#  - clients randomly shuffle their peer list
#  - skip to next peer if download of peer fails
#
# Revision 1.2  2009/09/15 21:52:13  bruno
# closer
#
# Revision 1.1  2009/09/08 22:17:49  bruno
# start of the new tracker
#
#

REDHAT.ROOT     = $(CURDIR)/../../

-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

INCLUDE	= -I/usr/include/apr-1
#LIBS	= -L/opt/rocks/lib -lcurl
#LIBS	= -lcurl -lssl -lidn -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lpthread -lcrypto -lz -ldl -lcom_err -lkeyutils -lresolv -lselinux -lsepol
LIBS	= -lcurl
#EXTRA	= -Wall
EXTRA	= -Wall -DDEBUG -pg -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

build:	tracker-client.cgi server stop-server

tracker-client.cgi:	tracker-client.c client.c lib.c
	cc $(INCLUDE) $(EXTRA) -o tracker-client.cgi tracker-client.c \
		client.c lib.c $(LIBS)

server:		server.o lib.o
	cc $(INCLUDE) $(EXTRA) -o server server.o lib.o $(LIBS)

lib.o:	lib.c
	cc $(INCLUDE) $(EXTRA) -c lib.c

server.o:	server.c
	cc $(INCLUDE) $(EXTRA) -c server.c

hashit:		hashit.c
	cc $(INCLUDE) $(EXTRA) -o hashit hashit.c lib.c $(LIBS)

stop-server:	stop-server.c
	cc $(INCLUDE) $(EXTRA) -o stop-server stop-server.c client.c lib.c \
		$(LIBS)

install::
	mkdir -p $(ROOT)/$(PKGROOT)/bin
	cp tracker-client.cgi $(ROOT)/$(PKGROOT)/bin
	cp stop-server $(ROOT)/$(PKGROOT)/bin
	cp server $(ROOT)/$(PKGROOT)/bin

clean::
	rm -f $(NAME).spec.in
	rm -f tracker-client.cgi server hashit stop-server
	rm -f *.o


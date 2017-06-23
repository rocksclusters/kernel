#!/bin/sh
# start lighttpd 
LIGHTROOT=/lighttpd
EXECUTABLE=$LIGHTROOT/sbin/lighttpd
CONF=$LIGHTROOT/conf/lighttpd.conf
TRACKERROOT=/tracker
CLIENTCONF=/tmp/rocks.conf
# Make sure we have a tracker configuration
if [ ! -f $CLIENTCONF ]; then
	cat /proc/cmdline | awk -f /tracker/tracker.ak  > $CLIENTCONF
fi

NZCONF=$(cat $CLIENTCONF)
if [ "x$NZCONF" == "x" ]; then return 0; fi

# check if lighttpd is already running. if so. We're done
ps ax | grep lighttpd | grep -q -v grep 
if [ $? -eq 0 ]; then return 0; fi
 
if [ ! -d /run/tracker ]; then
	mkdir /run/tracker
fi;

if [ ! -d /install ]; then
	ln -s /run/tracker /install		
fi;

## Start up lighttpd
if [ -x $EXECUTABLE -a -f $CONF ]; then
	$EXECUTABLE -f $CONF
fi

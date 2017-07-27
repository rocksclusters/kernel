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
if [ "x$NZCONF" == "x" ]; then /bin/rm $CLIENTCONF; fi


#!/bin/sh
# start lighttpd 
LIGHTROOT=/lighttpd
EXECUTABLE=$LIGHTROOT/sbin/lighttpd
CONF=$LIGHTROOT/conf/lighttpd.conf
if [ -x $EXECUTABLE -a -f $CONF ]; then
	$EXECUTABLE -f $CONF
fi

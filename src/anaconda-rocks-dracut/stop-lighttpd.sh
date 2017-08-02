#!/bin/sh
# Find lighttpd process and send it a SIGINT to complete existing connections
ID=$(ps ax | grep lighttpd | grep -q -v grep | awk '{print $1}')
if [ "x$ID" != "x" ]; then 
	kill -INT $ID
fi
return 0

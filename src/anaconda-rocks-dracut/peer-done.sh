#!/bin/sh
# Tell the tracker to unregister everything.
# This is called if we perform a shutdown in the initial ram disk
TRACKERHOME=/tracker
if [ -x $TRACKERHOME/peer-done ];  then 
	$TRACKERHOME/peer-done	
fi
return 0

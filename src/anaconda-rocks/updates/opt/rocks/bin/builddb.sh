#! /bin/bash 
# initialize the rocks db for in memory during graphical frontend installation
# Hacky bootstrap if given the core roll iso
# Creates in local directory
WHERE=$1
HERE=`pwd`
ROCKS=/opt/rocks/bin/rocks
RPM2CPIO=rpm2cpio
MYNAME=$(hostname -s)

# Get the nodes/graphs files from the roll itself. These  will be put under
# $CURDIR/export/profile
cd $WHERE
for roll in contrib/*/*/*/roll-*-kickstart-*.noarch.rpm; do 
    $RPM2CPIO $roll | cpio -id 
done

# Now bootstrap the database
if [ ! -d /var/opt/rocks/mysql ]; then
	mkdir -p /var/opt/rocks/mysql
fi
NODES=export/profile/nodes
tmpfile=$(/bin/mktemp)
/bin/cat $NODES/database.xml $NODES/database-schema.xml $NODES/database-sec.xml | $ROCKS report post attrs="{'hostname':'', 'HttpRoot':'/var/www/html','os':'linux'}"  > $tmpfile
if [ $? != 0 ]; then echo "FAILURE to create script for bootstrapping the Database"; exit -1; fi
/bin/sh $tmpfile
# /bin/rm $tmpfile
MYNAME=`hostname -s`
$ROCKS add distribution rocks-dist
$ROCKS add appliance bootstrap node=server
$ROCKS add host $MYNAME rack=0 rank=0 membership=bootstrap


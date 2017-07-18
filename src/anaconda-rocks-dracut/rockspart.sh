#!/bin/bash
PARTFILE="/tmp/partition-info"
/opt/rocks/bin/python /opt/rocks/lib/python2.7/site-packages/rocks/do_partition.py > $PARTFILE

#! /bin/bash
#
# provides can confuse apps that are looking for libsqlite provided 
# by system sqlite rpm 
/usr/lib/rpm/find-provides $* | sed -e 's/libsqlite/foundation-libsqlite/g' | sed -e '/^[[:space:]]*$/d' | sed -e '/^#/d'


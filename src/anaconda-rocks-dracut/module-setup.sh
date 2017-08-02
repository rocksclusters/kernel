#!/bin/bash
# module-setup.sh for rocks 
# This makes certain that we have tracker and lighttpd in the ramdisk
#


check() {
    [[ $hostonly ]] && return 1
    return 0 
}

depends() {
    echo livenet ifcfg
    return 0
}

install() {
    # binaries we want in initramfs
    # tracker/lightttpd
    for i in $(find /tracker -type f); do 
    	inst_binary $i 
    done
    for i in $(find /lighttpd -type d); do 
    	inst_binary $i 
    done
    for i in $(find /lighttpd -type f); do 
    	inst_binary $i 
    done
    # get the fetchRocksKS.py script
    inst_binary /fetchRocksKS.py

    # bring up lighttpd in initrd. Can get and cache installer,
    # xml files, pkgs. 
    inst_hook initqueue/online 10 "$moddir/start-lighttpd.sh"

    # gracefully stop lighttpd when transitioning to real root
    inst_hook pre-pivot 20 "$moddir/stop-lighttpd.sh"

    # unregister all files and then gracefully stop lighttpd 
    # when shutting down 
    inst_hook shutdown 19 "$moddir/peer-done.sh"
    inst_hook shutdown 20 "$moddir/stop-lighttpd.sh"
}


#!/bin/bash
# module-setup.sh for rocks 


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
    for f in tracker-client peer-done unregister-file; do
         inst_binary /opt/rocks/bin/$f /tracker/$f
    done
    for i in $(find /lighttpd -type d); do 
    	inst_binary $i 
    done
    for i in $(find /lighttpd -type f); do 
    	inst_binary $i 
    done
    # support kickstart fetch 
    inst_hook initqueue/online 10 "$moddir/start-lighttpd.sh"
}


REDHAT.ROOT     = $(PWD)/../../../../../../
ROCKSROOT       = ../../../../../../../../..

-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk


pretar::
	dd if=/dev/zero of=updates.img bs=1024 count=1024
	echo y | /sbin/mkfs -t ext2 updates.img
	mkdir mnt
	mount -o loop updates.img mnt
	mkdir -p mnt/pixmaps
	cp *png mnt/pixmaps
	umount mnt

clean::
	rm -f updates.img
	rm -f $(SPECFILE).in
	rm -rf mnt


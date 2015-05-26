TOP = $(shell pwd)
TEK_ZIP = MSODPO2000_V1_56.zip
UNZIP = unzip
MOUNT = sudo mount -oloop
UNMOUNT = sudo umount
TAR = sudo tar

phony := 
clean :=

all:
phony += all

unpack_zip:
	-rm -rf $(TOP)/unzipped && mkdir -p $(TOP)/unzipped
	cd $(TOP)/unzipped && $(UNZIP) $(TOP)/$(TEK_ZIP)
phony += unpack_zip
clean += $(TOP)/unzipped

mount_firmware:
	mkdir -p $(TOP)/firmware && \
	$(MOUNT) $(TOP)/unzipped/firmware.img $(TOP)/firmware
phony += mount_firmware

unmount_firmware:
	$(UNMOUNT) $(TOP)/firmware && rmdir $(TOP)/firmware
phony += unmount_firmware

unpack_rootfs:
	mkdir -p $(TOP)/rootfs && \
		cd $(TOP)/rootfs && \
		$(TAR) xf $(TOP)/firmware/filesystem.tar.gz 
clean += $(TOP)/rootfs
phony += unpack_rootfs

build_hook:
	$(MAKE) -C $(TOP)/src
phony += build_hook

copy_hook:
	sudo install --mode=0775 $(TOP)/src/override.so $(TOP)/rootfs/usr/local/bin
	sudo install --mode=0775 $(TOP)/src/scopeApp.sh $(TOP)/rootfs/usr/local/bin
	sudo install --mode=0640 $(TOP)/src/inittab $(TOP)/rootfs/etc/inittab
phony += copy_hook

patch_scope:
	sudo $(TOP)/tools/binpatch.sh $(TOP)/rootfs/usr/local/bin/scopeApp.ppc8xx -yes
phony += patch_scope

pack_rootfs:
	cd $(TOP)/rootfs && \
		sudo tar czf $(TOP)/firmware/filesystem.tar.gz .
phony += pack_rootfs

update_md5:
	cd $(TOP)/firmware && \
		sudo $(TOP)/tools/update_md5.sh
phony += update_md5

copy_firmware:
	cp $(TOP)/unzipped/firmware.img $(TOP)/firmware_patched.img
phony += copy_firmware


banner:
	@echo 
	@echo "You can now put file firmware_patched.img to your usb flash (as firmware.img)"
	@echo "and try to use it to flash your scope. Good luck!"

clean: unmount_firmware
	$(MAKE) -C $(TOP)/src clean
	-sudo rm -rf $(clean)
phony += clean

all: \
	unpack_zip \
	mount_firmware \
	unpack_rootfs \
	build_hook \
	copy_hook \
	patch_scope \
	pack_rootfs \
	update_md5 \
	unmount_firmware \
	copy_firmware \
	clean \
	banner

.PHONY: $(phony)

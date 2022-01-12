#!/bin/sh

path="$(dirname $0)/../../"
mkdir -p tmp/boot

cp $path/build/kernel.elf ./tmp/boot
cp -r $path/src/root/* ./tmp
cd ./tmp
tar -cf root.tar *
mv root.tar boot
cd ..

cp $path/thirdparty/limine/limine.sys ./tmp/boot
cp $path/thirdparty/limine/*.bin ./tmp/boot

xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size $(nproc --all) \
	-boot-info-table --efi-boot limine-eltorito-efi.bin -efi-boot-part --efi-boot-image \
	--protective-msdos-label ./tmp/boot -o $path/bestOS.iso

$path/thirdparty/limine/limine-install-linux-x86_64 $path/bestOS.iso
rm -rf ./tmp

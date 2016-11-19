echo "Ensuring VM is disabled"
Stop-VM -Name LunOS -TurnOff
echo "Mounting the vhd for LunOS"
Mount-DiskImage C:\Users\phibr\Documents\LunOS\VHD\LunOS.vhd
bash -c make
echo "Copying kernel to disk"
cp kernel.elf e:/boot/
echo "Dismounting the Disk"
DisMount-DiskImage C:\Users\phibr\Documents\LunOS\VHD\LunOS.vhd
echo "Disk is ready for boot"
echo "Starting Virtual Machine"
Start-VM -Name LunOS

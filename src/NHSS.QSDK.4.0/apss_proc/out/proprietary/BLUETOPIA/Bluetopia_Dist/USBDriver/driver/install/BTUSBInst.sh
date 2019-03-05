#!/bin/sh

# By defalut the driver path is the current directory.
DRIVER_PATH=$(pwd)

install()
{
	# Create a new udev rule file
	# This rule file will run the bind_script and change the default permisions
	# for the device created.
	echo "DRIVER==\"btusb\", RUN+=\"$DRIVER_PATH/bind_driver\"" > /etc/udev/rules.d/10-SS1BTUSB.rules
	echo "KERNEL==\"SS1BTUSB[0-9]\", 	GROUP=\"root\", MODE=\"0666\"" >> /etc/udev/rules.d/10-SS1BTUSB.rules
	
	# Replace the driver path in the bind_driver script
	sed -i "s|DRIVER_PATH=.*|DRIVER_PATH="$DRIVER_PATH"|g" bind_driver

	./bind_driver
}

uninstall()
{
	# Remove the module if it is in memory
	rmmod SS1BTUSBM.ko > /dev/null 2>&1
	
	# Delete the udev rule file

	if [ -e /etc/udev/rules.d/10-SS1BTUSB.rules ]
	then
		echo "Deleting the udev rules"
		rm /etc/udev/rules.d/10-SS1BTUSB.rules
	fi
}

# Root or die
if [ "$(id -u)" != "0" ]
then
	echo "You must be root"
	exit 1
fi

case "$1" in
	install)
	install
	;;
	uninstall)
	uninstall
	;;
	*)
	echo "Usage: $0 (install | uninstall)"
	exit 1
esac

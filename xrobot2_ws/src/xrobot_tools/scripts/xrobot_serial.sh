#!/bin/bash
echo  'KERNEL=="ttyUSB*", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6011", MODE:="0666", GROUP:="dialout"' >/etc/udev/rules.d/99-xrobot.rules
echo  'KERNEL=="ttyACM*", GROUP:="dialout"' >/etc/udev/rules.d/99-xrobot.rules

service udev reload
sleep 2
service udev restart

#!/bin/bash

echo ""
echo "This script copies a udev rule to /etc"
echo ""

sudo cp `rospack find astra_camera`/56-orbbec-usb.rules /etc/udev/rules.d
sudo cp `rospack find xrobot_tools`/startup/99-xrobot.rules /etc/udev/rules.d


echo ""
echo "Restarting udev"
echo ""
sudo service udev reload
sudo service udev restart
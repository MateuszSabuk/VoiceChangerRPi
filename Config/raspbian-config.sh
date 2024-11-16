#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

apt update -y || exit
apt upgrade -y || exit

apt install python3-venv
python -m venv env --system-site-packages

source env/bin/activate

sudo -E env PATH=$PATH python3 adafruit-script.py


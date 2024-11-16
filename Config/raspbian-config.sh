#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

apt update -y || exit
apt upgrade -y || exit

apt install jackd2 libjack-jackd2-dev

g++ ../Code/main.cpp -o ff.o -l jack && sudo cp ff.o /usr/local/sbin/ff

apt install python3-venv
python -m venv env --system-site-packages

source env/bin/activate
pip install adafruit-python-shell

sudo -E env PATH=$PATH python3 adafruit-script.py
sudo reboot

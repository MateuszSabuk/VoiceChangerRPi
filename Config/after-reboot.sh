#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi



string=`arecord -l | grep "googlevoicehat_soundcar"`
if [[ "$string" =~ ^card[[:space:]]([0-9]+).+?googlevoicehat_soundcar.+?,[[:space:]]device[[:space:]]([0-9]+?): ]]; then
    card=${BASH_REMATCH[1]}
    device=${BASH_REMATCH[2]}
    
    sudo usermod -aG audio `whoami`
    sudo echo """[Unit]
Description=JACK Audio Connection Kit
After=sound.target

[Service]
Group=audio
User=`whoami`
Environment=JACK_NO_AUDIO_RESERVATION=1
ExecStart=/usr/bin/jackd -R -d alsa -d hw:$card,$device
Restart=on-failure
LimitRTPRIO=95
LimitMEMLOCK=infinity

[Install]
WantedBy=default.target""" > /etc/systemd/system/jack.service
    
else
    echo "No device found"
fi
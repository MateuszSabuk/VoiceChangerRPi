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
     
    sudo echo """[Unit]
Description=Simple feed forward from mic to speaker
After=jack.service

[Service]
Group=audio
User=`whoami`
ExecStart=/usr/bin/ff
Restart=on-failure

[Install]
WantedBy=default.target""" > /etc/systemd/system/soundlooptest.service
     
    sudo systemctl daemon-reload
    sudo systemctl enable jack
    sudo systemctl start jack
    sudo systemctl enable soundlooptest
    sudo systemctl start soundlooptest


else
    echo "No device found"
fi
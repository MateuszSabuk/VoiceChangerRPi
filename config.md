# Raspberry Pi 4B config

Installed [PatchboxOS](https://blokas.io/patchbox-os/)

1. exited the basic configuration

2. Run the commands in the /home/patch directory

```bash
sudo apt update -y
apt upgrade -y

apt install python3-venv git

git clone https://github.com/MateuszSabuk/voicechangerrpi
cd voicechangerrpi
python -m venv env --system-site-packages

source env/bin/activate
sudo pip install -r requirements.txt

sudo -E env PATH=$PATH python3 config-script.py

sudo echo """[Unit]
Description=Simple feed forward from mic to speaker

[Service]
User=patch
ExecStart=/home/patch/voicechangerrpi/env/bin/python3 /home/patch/voicechangerrpi/run.py
Restart=on-failure

[Install]
WantedBy=default.target""" > /etc/systemd/system/voicechanger.service

sudo systemctl daemon-reload
sudo systemctl enable voicechanger
sudo systemctl start voicechanger

mkdir /home/patch/.config/autostart
echo """[Desktop Entry]
Type=Application
Name=SCServer
Exec=lxterminal -e sclang /home/patch/voicechangerrpi/effects.scd
""" > /home/patch/.config/autostart/scserver.desktop

sudo nmcli device wifi hotspot con-name VoiceChangerMati ssid VoiceChangerMati band bg password Password
sed -i 's/autoconnect=false/autoconnect=true/' /etc/NetworkManager/system-connections/VoiceChangerMati.nmconnection
```

3. Run the patchbox command and turn on the jack realtime
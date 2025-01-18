# Raspberry Pi 4B config

Installed [PatchboxOS](https://blokas.io/patchbox-os/)

1. exited the basic configuration

2. Run the commands in the /home/patch directory

```bash
sudo apt update -y
apt upgrade -y

apt install python3-venv git # dnsmasq iptables iptables-persistent

git clone https://github.com/MateuszSabuk/voicechangerrpi
cd voicechangerrpi
python -m venv env --system-site-packages

env/bin/pip install -r requirements.txt

sudo -E env PATH=$PATH env/bin/python3 config-script.py

sudo echo """[Unit]
Description=Simple feed forward from mic to speaker

[Service]
User=patch
ExecStart=/home/patch/voicechangerrpi/env/bin/python3 /home/patch/voicechangerrpi/run.py
Restart=on-failure

[Install]
WantedBy=default.target""" | sudo tee /etc/systemd/system/voicechanger.service

sudo systemctl daemon-reload
sudo systemctl enable voicechanger
sudo systemctl start voicechanger

mkdir /home/patch/.config/autostart
echo """[Desktop Entry]
Type=Application
Name=SCServer
Exec=lxterminal -e sclang /home/patch/voicechangerrpi/effects.scd
""" | tee /home/patch/.config/autostart/scserver.desktop

```

1. Run the patchbox command
   1. Turn on the hotspot
   2. Turn on default login to desktop
   3. Turn on realtime kernel
   (sudo reboot)
   4. Set the jack default device to sndrpigoogle

```bash
sudo sed -i 's/address1=172.24.1.1/address1=10.0.0.1/' /etc/NetworkManager/system-connections/pb-hotspot.nmconnection

# # Maybe someday will make this part work for caption portal
# echo """interface=wlan0
# dhcp-range=10.0.0.2,10.0.0.100,255.255.255.0,24h
# address=/#/10.0.0.1  # Redirect all DNS requests to Raspberry Pi
# """ | sudo tee /etc/dnsmasq.conf

# sudo sysctl -w net.ipv4.ip_forward=1
# echo "net.ipv4.ip_forward=1" | sudo tee -a /etc/sysctl.conf
# sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-port 5000

# sudo reboot
```



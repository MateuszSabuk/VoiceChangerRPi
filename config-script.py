# Config script from adafruit

import os

try:
    from adafruit_shell import Shell
    from clint.textui import colored
except ImportError:
    raise RuntimeError("The library 'adafruit_shell' was not found. To install, try typing: sudo pip3 install adafruit-python-shell")

shell = Shell()

BLACKLIST = "/etc/modprobe.d/raspi-blacklist.conf"
PRODUCT_NAME = "I2S Amplifier"

def driver_loaded(driver_name):
    return shell.run_command(f"lsmod | grep -q '{driver_name}'", suppress_message=True)

def main():
    shell.clear()
    if not shell.is_raspberry_pi():
        shell.bail("Non-Raspberry Pi board detected.")
    print("\nThis script will install everything needed to use\n"
        f"{PRODUCT_NAME}.\n")

    print("\nChecking hardware requirements...")

    # Enable I2S overlay
    config = "/boot/firmware/config.txt"
    if not os.path.exists(config):
        config = "/boot/config.txt"
    if not os.path.exists(config):
        shell.bail("No Device Tree Detected, not supported")

    print(f"\nAdding Device Tree Entry to {config}")

    if shell.pattern_search(config, "^dtparam=i2s=on$"):
        print("dtparam=i2s already on")
    else:
        shell.write_text_file(config, "dtparam=i2s=on")

    # Add Speaker
    if shell.pattern_search(config, "^dtoverlay=max98357a$"):
        print("Speaker already active")
    else:
        shell.write_text_file(config, "dtoverlay=max98357a")
    
    # Add Microphone
    if shell.pattern_search(config, "^dtoverlay=googlevoicehat-soundcard$"):
        print("Mic already active")
    else:
        shell.write_text_file(config, "dtoverlay=googlevoicehat-soundcard")

    if os.path.exists(BLACKLIST):
        print("\nCommenting out Blacklist entry in", BLACKLIST)
        shell.pattern_replace(BLACKLIST, "^blacklist[[:space:]]*snd_soc_max98357a.*", "#blacklist snd_soc_max98357a")
        shell.pattern_replace(BLACKLIST, "^blacklist[[:space:]]*snd_soc_max98357a_i2c.*", "#blacklist snd_soc_max98357a_i2c")
        shell.pattern_replace(BLACKLIST, "^blacklist[[:space:]]*snd_soc_max98357a.*", "#blacklist snd_soc_max98357a")

    print("Configuring sound output")
    if os.path.exists("/etc/asound.conf"):
        if os.path.exists("/etc/asound.conf.old"):
            shell.remove("/etc/asound.conf.old")
        shell.move("/etc/asound.conf", "/etc/asound.conf.old")
    shell.write_text_file("~/asound.conf",
"""
pcm.speakerbonnet {
   type hw card 0
}

pcm.dmixer {
   type dmix
   ipc_key 1024
   ipc_perm 0666
   slave {
     pcm "speakerbonnet"
     period_time 0
     period_size 1024
     buffer_size 8192
     rate 44100
     channels 2
   }
}

ctl.dmixer {
    type hw card 0
}

pcm.softvol {
    type softvol
    slave.pcm "dmixer"
    control.name "PCM"
    control.card 0
}

ctl.softvol {
    type hw card 0
}

pcm.!default {
    type             plug
    slave.pcm       "softvol"
}
""")
    shell.move("~/asound.conf", "/etc/asound.conf")


    print("Installing aplay systemd unit")
    shell.write_text_file("/etc/systemd/system/aplay.service", """
[Unit]
Description=Invoke aplay from /dev/zero at system start.

[Service]
ExecStart=/usr/bin/aplay -D default -t raw -r 44100 -c 2 -f S16_LE /dev/zero

[Install]
WantedBy=multi-user.target""", append=False)

    shell.run_command("sudo systemctl daemon-reload")
    shell.run_command("sudo systemctl enable aplay")

    if driver_loaded("max98357a"):
        print("Testing...")
        shell.run_command("speaker-test -l5 -c2 -t wav")
    print("\n" + colored.green("All done!"))

# Main function
if __name__ == "__main__":
    shell.require_root()
    main()
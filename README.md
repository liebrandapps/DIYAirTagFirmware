# DIYAirTagFirmware

This is a firmware for a NRF51822 based beacon creating a DIY AirTag. 

The code is based on

https://github.com/acalatrava/openhaystack-firmware.git
https://github.com/dakhnod/FakeTag.git

The firmware support sending the battery level through the status byte. The onboard switch (can be replaced by a vibration sensor) increases a counter in the status field.

The status bytes is part of the location information that is received from the Apple servers.

## Setting up

Get the sublmodules

```bash
git submodule init
git submodule update
```

## Other things you need

You will require a cross compiler for the nrf51822. Below commands for a Raspberry Pi running Raspbian. 

```bash
wget https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-24-0/nrf-command-line-tools_10.24.0_amd64.deb

dpkg -i nrf-command-line-tools_10.24.0_amd64.deb

apt install gcc-arm-none-eab
```

## Building

Go to the apps folder

```bash
export NRF_MODEL=nrf51; make build

export NRF_MODEL=nrf51; export ADV_KEY_BASE64=<YOUR BASE64 ADV KEY GOES HERE>; make patch
```




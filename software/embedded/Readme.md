Testeur software
================

To build the firmware, there is few thing you need to have.

First, Fetch ChibiOs and some third party contributions with `git submodule update --init --recursive`.

__Install Perl dependencies (used to generate board.h file from board.cfg):__

```
sudo apt update
sudo apt install libc6:i386 libncurses5:i386 libstdc++6:i386
sudo apt install libmodern-perl-perl libxml-libxml-perl
sudo perl -MCPAN -e 'install String::LCSS'
```

__Install GCC for ARM:__

Fetch the latest version from [https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads).

Extract it to `/usr/local` (or anywhere you want).

Make a symbolic link to it (adapt the command with the right version name):

```
cd /usr/local
ln -s  gcc-arm-none-eabi-7-2017-q4-major gcc-arm-none-eabi
```

Add it to your $PATH by adding this line either in your ~/.profile or ~./bashrc:

`PATH="$PATH:/usr/local/gcc-arm-none-eabi/bin"`

__Install STM32CubeMX__

Download and install SMT32CubeMx with admin rights (need to signup on st.com): [http://www.st.com/en/development-tools/stm32cubemx.html](http://www.st.com/en/development-tools/stm32cubemx.html).

Make sure it is installed in `/usr/local` for linux users (_not_ in your $HOME) or `C:\Program Files` for windows users.

__Edit udev rules for Black Magic Probe__

If you are using the nucleo with black magic probe, install the udev rule :

`sudo cp 49-black_magic_probe.rules /etc/udev/rules.d/`

and update them with :

`sudo udevadm control --reload-rules && sudo udevadm trigger`

__Build__

To build the firmware, go to software/embedded then run `make`.

To flash the firmware, copy paste the .bin generated file to your nucleo.

If your nucleo StLink has been flashed with [BlackMagicProbe](https://github.com/blacksphere/blackmagic/), run `make flash`.


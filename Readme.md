Servo Tester
=============

_This project is a work in progress. Nothing is yet stable, and a lot of features are missing._

A device to quickly command servomoteurs and other stuff.

Based on a nucleo32 L432KC, this device allow to command up to 5 (only 3 working for now) classic servomotors.
It also support dynamixel AX-12A servomotors.

It features I2C and CAN capabilities. 


## Hardware

PCB was made with Kicad 5.0. Go to the Release page to get the gerbers files.

Run `git submodule update --init --recursive` to fetch kicad libraries.

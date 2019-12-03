Servo Tester
=============
[![Build Status](https://github.com//Fabien-B/Servo_Tester/workflows/Embedded%20STM32%20Build/badge.svg)](https://github.com/Fabien-B/Servo_Tester/actions?query=workflow%3A%22Embedded+STM32+Build%22)

_This project is a work in progress. Some features are missing._

A device to quickly command servomoteurs and other stuff like I2C or CAN devices.

Based on a nucleo32 L432KC, this device allow to command up to 5 (only 3 working for now) classic servomotors.
It also support dynamixel AX-12A servomotors.


## Hardware

PCB was made with Kicad 5.0. Go to the Release page to get the gerbers files.

Run `git submodule update --init --recursive` to fetch kicad libraries.

## Software

The embedded firware uses ChibiOs.

For instructions about embedded firmware go to the appropriate [Readme file](software/embedded/Readme.md).


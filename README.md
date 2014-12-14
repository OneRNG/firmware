firmware
========

This repository contains firmware for the OneRNG entropy generator

To build this you will need a copy of the same version of the open source SDCC compiler 
that we use to build the system - you can find one here

  http://moonbaseotago.com/cheaprf/sdcc-2533.tar.gz
  
You can find more documentation about setting up a build sustem and programming devices here:

  http://www.moonbaseotago.com/cheaprf/#programming

The code contained in this directory is a fork of the MoonbaseOtago CheapRF code - it has 
had the RF and SUOTA bits removed for obvious security reasons, you can look at and download 
the original code in its repository here:

  https://github.com/MoonbaseOtago/CC-System

Documentation about the minimal OS and how to use it is here:

  http://www.moonbaseotago.com/cheaprf/programming.html
  
The one main difference is that the thread schedular instead of low power idling the CPU 
it runs the entropy gathering idle_pool() routine 

Files:

  Makefile    - type 'make' to build - change SDCC_INSTALL_DIR to point to where your compiler is installed
  usb.c       - cc2531 USB stack
  cdc.c       - USB decsriptors - handler stubs for CDC messages
  cdc_app.c   - main entropy gathering code (the interesting stuff is here)
  kernel/*    - task scheuler/kernel support
  include/*   - kernel incloude files
  
The build system works like this:

1) build a lodable app cdc_app.ihx (intel hex format - you can just use this if you are rolling your own)
2) concatenate the memory image of the code with known random_data, truncate the result to 
   the ROM size less room for a signing
3) sign the resulting image with our private key (not provided)
4) pad the remaining code with FFs so that the chip is not code locked (open source and binaries right?)

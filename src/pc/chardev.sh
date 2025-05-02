#!/bin/bash
make chardev
sudo insmod chardev.ko
sudo chmod 0666 /dev/tiziano_chardev0


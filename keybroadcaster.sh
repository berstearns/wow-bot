#!/bin/bash

while [ 1 ]; do
    wmctrl -i -a 0x02200003   #forcibly set focus in window running script
    read keys
    xvkbd -window 0x2202ea4 -text "$keys\r"
    xvkbd -window 0x2200084 -text "$keys\r"
done

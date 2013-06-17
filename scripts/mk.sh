#!/bin/bash
gcc client.c network.c  -o weatherclient || exit
gcc -lm -ggdb parser.c datautils.c network.c -o parser || exit
gcc conftool.c network.c -o conftool || exit

scp conftool surf:cgi-bin/
scp parser surf:bin/
scp weatherclient surf:bin/

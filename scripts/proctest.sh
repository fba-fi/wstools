#!/bin/bash

CLIENT="`pgrep weatherclient`"

if [ -n "$CLIENT" ] ; then
	echo "toimi"
else
	echo "poissa"
fi

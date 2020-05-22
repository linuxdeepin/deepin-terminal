#!/bin/bash

touch views/termtabbar.moc
cd terminalwidget
if [ -d build ]
then
	rm -fr build
fi
mkdir build
cd build
cmake ..
make
ENV_STRING="export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$PWD"
BASH_RC_FILE=$HOME/.bashrc
if [ ! `grep -c "$ENV_STRING" $BASH_RC_FILE` -ne '0' ];then
    echo "$ENV_STRING" >> $BASH_RC_FILE
fi


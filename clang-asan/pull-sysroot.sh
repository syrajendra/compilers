#!/bin/sh

# remote machine
RMACHINE=$1

# passwdless access
cat ~/.ssh/id_rsa.pub | ssh $RMACHINE 'cat >> ~/.ssh/authorized_keys'

# find out the target triple
TRIPLE=`ssh $RMACHINE "clang -dumpmachine"`
mkdir -p $TRIPLE
cd $TRIPLE
rsync -arvz 192.168.1.21:/lib .
rsync -arvz 192.168.1.21:/libexec .
mkdir usr
cd usr
rsync -arvz 192.168.1.21:/usr/include .
rsync -arvz 192.168.1.21:/usr/libexec .
rsync -arvz 192.168.1.21:/usr/libdata .
rsync -arvz 192.168.1.21:/usr/lib .



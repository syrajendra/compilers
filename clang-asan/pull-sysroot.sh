#!/bin/sh

# remote machine
RMACHINE=$1

# passwdless access
cat ~/.ssh/id_rsa.pub | ssh $RMACHINE 'cat >> ~/.ssh/authorized_keys'

# find out the target triple
TRIPLE=`ssh $RMACHINE "clang -dumpmachine"`
mkdir -p $TRIPLE
cd $TRIPLE
rsync -arvz $RMACHINE:/lib .
rsync -arvz $RMACHINE:/libexec .
mkdir usr
cd usr
rsync -arvz $RMACHINE:/usr/include .
rsync -arvz $RMACHINE:/usr/libexec .
rsync -arvz $RMACHINE:/usr/libdata .
rsync -arvz $RMACHINE:/usr/lib .



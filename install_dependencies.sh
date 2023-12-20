#!/bin/bash
sudo bash -c "echo deb [arch=amd64] http://archive.ubuntu.com/ubuntu focal main universe >> /etc/apt/sources.list"
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y
sudo apt install build-essential g++ python3-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev -y
sudo apt install unzip
wget http://download.oracle.com/berkeley-db/db-4.8.30.zip
unzip db-4.8.30.zip
cd db-4.8.30
cd build_unix/
../dist/configure --prefix=/usr/local --enable-cxx
sudo make
sudo make install
cd ../..
sudo rm -rf db-4.8.30 db-4.8.30.zip
sudo apt install libssl-dev -y
sudo apt install libglib2.0-dev -y
sudo cp /usr/lib/x86_64-linux-gnu/{libglib-2.0.a,libglib-2.0.so} /usr/lib
sudo cp libs/liblucene++.so libs/liblucene++-contrib.so /usr/lib/
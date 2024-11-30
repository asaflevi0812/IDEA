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
# added in order to solve ambiguity
sed -i 's/__atomic_compare_exchange/__atomic_compare_exchange_db/g' db-4.8.30/dbinc/atomic.h

cd db-4.8.30
cd build_unix/
../dist/configure --prefix=/usr/local --enable-cxx
sudo make
sudo make install
cd ../..
#sudo rm -rf db-4.8.30 db-4.8.30.zip
sudo apt install libssl-dev -y
sudo apt install libglib2.0-dev -y
sudo cp /usr/lib/x86_64-linux-gnu/{libglib-2.0.a,libglib-2.0.so} /usr/lib
sudo cp libs/liblucene++.so libs/liblucene++-contrib.so /usr/lib/

# added to solve compilation issues
sudo apt install g++-9
sudo apt-get install cmake

# setting correct boost version:
# it is really important to delete any traces of boost that may confuse the linker
sudo apt-get -y --purge remove libboost-all-dev libboost-doc libboost-dev
sudo rm -f /usr/lib/libboost_*
sudo rm /usr/local/lib/libboost*
sudo rm -rf /usr/local/include/boost/
sudo apt remove --purge 'libboost*'
wget http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz
tar -xvf boost_1_58_0.tar.gz
cd boost_1_58_0/
./bootstrap.sh
./b2
sudo ./b2 install
cd ../

# setting correct lucene version with boost support:
# it is really important to delete any traces of lucene that may confuse the linker
sudo apt-get remove --purge lucene
sudo apt remove --purge lucene++
sudo rm /usr/lib/liblucene*
git clone https://github.com/luceneplusplus/LucenePlusPlus.git
cd LucenePlusPlus
# sets desired boost directory. Can be changed based on installation
cmake -DBOOST_ROOT=/usr/local -DBOOST_INCLUDEDIR=/usr/local/include -DBOOST_LIBRARYDIR=/usr/local/lib -DCMAKE_CXX_FLAGS="-I/usr/local/include -L/usr/local/lib" .
# fixes include issue
sudo sed -i '10s|#include <boost/filesystem/directory.hpp>|#include <boost/filesystem.hpp>|' src/core/util/FileUtils.cpp
make
sudo make install
cd ../

###################
# SETUP & INSTALL #
###################

sudo apt update

# g++-7
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y
# boost
sudo apt-get install build-essential g++ python3-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev -y
# berkeley db
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
# openssl
sudo apt-get install libssl-dev -y
# glib
sudo apt-get install libglib2.0-dev -y
# git
sudo apt install git -y
# fix glib path
sudo cp /usr/lib/x86_64-linux-gnu/{libglib-2.0.a,libglib-2.0.so} /usr/lib
# install cmake
sudo apt install cmake -y


########################################
# BUILD: copy code and build artifacts #
########################################

git clone https://github.com/asaflevi0812/IDEA.git
cd IDEA
# install LucenePlusPlus
sudo cp libs/liblucene++.so libs/liblucene++-contrib.so 
chmod +x compile.sh
./compile.sh
mkdir -p working_directories/backup_directory working_directories/index_directory working_directories/chunk_to_file_directory

#########################################
# RUN: example backup, index and lookup #
#########################################

# backup
destor datasets/LNX-3
# naive index
destor -n
# IDEA index
destor -q
# lookup the word "hello" in the naive index
destor -m hello
# lookup the word "hello" in the IDEA index
destor -l hello

#############
# CONFIGURE #
#############

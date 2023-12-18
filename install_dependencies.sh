sudo apt update
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y
sudo apt-get install build-essential g++ python3-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev -y
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
sudo apt-get install libssl-dev -y
sudo apt-get install libglib2.0-dev -y
sudo cp /usr/lib/x86_64-linux-gnu/{libglib-2.0.a,libglib-2.0.so} /usr/lib
sudo apt install git -y

This repository contains the source code for the paper:

#### Asaf Levi, Philip Shilane, Sarai Sheinvald, Gala Yadgar. Physical vs. Logical Indexing with IDEA: Inverted Deduplication-Aware Index. To appear in 22nd USENIX Conference on File and Storage Technologies (FAST '24), February 2024. 

Our implementation of the naïve and the deduplication-aware index is based on the Destor open-source storage system: https://github.com/fomy/destor

System requirements: Ubuntu version 16.04, and the list of dependencies installed by the script provided. 

# SETUP & INSTALL #
MAKE SURE TO USE A CLEAN UBUNTU SERVER 16.04 LTS IMAGE, IF POSSIBLE. (FROM [HERE](https://releases.ubuntu.com/16.04/ubuntu-16.04.7-server-amd64.iso))

An existing Ubuntu 16.04 machine is also OK. The `install_dependencies.sh` script is meant to run on a clean Ubuntu Server installation.

### install git and pull artifacts repository
Simply install git and clone the repo.
Dependencies are installed later in this README in ./install_dependencies.sh :)
```
# takes a couple of human minutes :)
sudo apt update
sudo apt install git -y
git clone https://github.com/asaflevi0812/IDEA.git
cd IDEA
```

### install dependencies
Installs the libraries required for IDEA:
* BerkeleyDB
* Lucene++
* BOOST
* SSL
* GLIB
```
# takes about a minute until a single [PRESS ENTER] is required. -- sometimes it does not.
# after that, about ten minutes of installation, probably less.
chmod +x install_dependencies.sh
./install_dependencies.sh
```

# BUILD: copy code and build artifacts #

### compile
RUN this from the IDEA repository to compile the code.
```
chmod +x compile.sh
./compile.sh
```

### prepare backup and index directories
In the destor.config file (discussed later), the paths to the directories containing the destor backup and the IDEA HDD index part and SSD index part are defined.

By default, they are defined as follows:
* backup directory: working_directories/backup_directory
* HDD part directory: working_directories/index_directory
* SSD part directory: working_directories/chunk_to_file_directory
the following command creates the directories:
```
mkdir -p working_directories/backup_directory working_directories/index_directory working_directories/chunk_to_file_directory
```
and in our original setup the proper devices are mounted to these directories.

# RUN: example backup, index and lookup #

### backup
`destor datasets/LNX-3`
### naive index
`destor -n`
### IDEA index
`destor -q`
### lookup the word "hello" in the naive index
`destor -m hello`
### lookup the word "hello" in the IDEA index
`destor -l hello`
### lookup with the file-med dictionary
```
destor -m -f"keywords/linux/128/file-med.txt"
destor -l -f"keywords/linux/128/file-med.txt"
```

# CONFIGURE #
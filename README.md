# SETUP & INSTALL #

### install git and pull artifacts repository
`sudo apt update`
`git clone https://github.com/asaflevi0812/IDEA.git`
`cd IDEA`

### install dependencies
`./install_dependencies.sh`

# BUILD: copy code and build artifacts #

### compile
```
chmod +x compile.sh
./compile.sh
mkdir -p working_directories/backup_directory working_directories/index_directory working_directories/chunk_to_file_directory
```

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
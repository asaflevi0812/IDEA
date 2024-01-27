This repository contains the source code for the paper:

##### Asaf Levi, Philip Shilane, Sarai Sheinvald, Gala Yadgar.
### Physical vs. Logical Indexing with IDEA: Inverted Deduplication-Aware Index.
##### To appear in 22nd USENIX Conference on File and Storage Technologies (FAST '24), February 2024. 

#

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

# BUILD & ENVIRONMENT #

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

The following command creates the directories:
```
mkdir -p working_directories/backup_directory working_directories/index_directory working_directories/chunk_to_file_directory
```
and in our original setup the proper devices are mounted to these directories.

# RUN: example backup, index and lookup #

### backup
The following command creates a deduplicated backup from a given dataset path: 

`destor <dataset_path>`

The entire directory will be recursively backed up in the backup directory defined in destor.config (discussed below).

### naive index
to create a naive index from the currently stored deduplicated backup - run the following command:

`destor -an`

The index will be stored in the HDD part index directory defined in destor.config (discussed below).

### IDEA index
to create an IDEA index from the currently stored deduplicated backup - run the following command:

`destor -aq`

The index parts will be stored accordingly the HDD part index directory and SSD part directory, both are defined in destor.config (discussed below).

### inline index
to create an inline index, which should backup the data and create the index concurrently, use the following commands:

`destor -n <path to data>`

for naive inline index and

`destor -q <path to data>`

for IDEA-indirect inline index.

### lookup keywords from the command line interface
use the command:

`destor -m|l <keyword1> <keyword2> <keyword3>`

where m is used for the naive index and l for the IDEA index.  For example

`destor -l hello world`

would lookup the keywords "hello" and "world" in the IDEA index.

### lookup a dictionary of keywords from a file
use the command:

`destor -m|l -f<path_to_dictionary>`

to search a list of keywords in an OR manner, i.e. if any of the terms are contained in a file, it would appear in the results.

For example use:

`destor -m -f"keywords/linux/128/file-med.txt"`

to search the file-med dictionary in the naive index.

# CONFIGURATION OPTIONS #
### destor.config
When destor is initialized, it reads a configuration file named in a fixed name, `destor.config`, which contains lines in the following format:

```
<configuration_variable> <value>
```

For example:
```
working-directory "/home/asaf.levi/destor_working_directory"
chunking-type whitespace
reverse-mapping db
```

The following values are used to configure parameters regarding IDEA and NAIVE:
* working-directory: path to a directory which will contain the deduplicated backups.
* index-directory: path to the HDD-part of an index. It will store different parts of the index according to the index type:
    * NAIVE: the entire index
    * IDEA: the term-to-file map
    * IDEA-indirect: the term-to-chunk map
* reverse-directory: path to the SSD-part of an index. It will store different parts of the index according to the index type:
    * IDEA: the file-to-path map
    * IDEA-indirect: the chunk-to-file and file-to-path maps
* chunking-type: can hold the values ` whitespace` or `whitespace-reversed`. The former is our default whitespace chunking and the latter is a chunking method used for fixed-sized blocks, where the next whitespace is searched before the chunk boundary and not after. More about these two methods in the paper.
* reverse-mapping: the type of chunk-to-file mapping. May be `in-doc` for IDEA and `db` for IDEA-indirect.
* indirect-path-strings: whether to enable file-to-path mapping. Always set to yes in our paper.
* offsets-mode: whether to store offsets. Supported values are `none` and `term-vectors`, where `term-vectors` is the only offsets option in the paper.
* tf-idf: whether to store tf-idf values. `yes` \ `no` value.

## POSSIBLE CONFIGURATIONS:
Several configurations where pre-created in the "configs" directory. These are the important values in these configurations:

#### IDEA
```
chunking-type whitespace
reverse-mapping in-doc
offsets-mode none
tf-idf no
```
#### IDEA-indirect
```
chunking-type whitespace
reverse-mapping db
offsets-mode none
tf-idf no
```
#### IDEA with offsets
```
chunking-type whitespace
reverse-mapping db
offsets-mode term-vectors
tf-idf no
```
#### IDEA with ranking
```
chunking-type whitespace
reverse-mapping db
offsets-mode none
tf-idf yes
```

# REPRODUCING RESULTS #
The results are best reproduced on a suitable clean Ubuntu 16.04 machine with only the required installations. If the use of SSDs and HDDs as described in the paper is possible, then the results should be more accurate.
NOTE: the figure scripts do not generate a graphical figure, they generate a CSV.

#### base indexing
[estimated runtime for LNX-198: 2 hours]

First, make sure the system is configured to create offset-less and rank-less indexes, and that the index working directories are empty.
You can do that with the following command:

```
scripts/clean_index.sh
cp configs/default.config destor.config
```

The script `scripts/create_indexes.sh` creates the basic IDEA and Naive index versions, cleaning the OS cache before each creation, and measures the creation time.
The output should indicate the start and end of the naive and deduplicated index building process. During the process, an updating progress bar appears with the number of chunks and files processed. After each index is built, several parameters’ values are printed. 
The script creates indexes as in *Figures 7+8* from the paper.
The indexes will be created in the working directories mentioned above.
The indexing time and index size for each index is logged by the script in the csv file _index.log_. Indexing time is in the *total_time* column and index size is in the *complete_index_size* column.

##### indexing with offsets or ranks:
[estimated runtime for LNX-198: 2.5-3 hours]

Use the "base indexing" guide with `offset.config` or `ranking.config` instead of `default.config`.


#### Figures from the paper
##### Figure 10
[estimated runtime for LNX-198: <2 minutes]

The script `scripts/figure_10.sh` performs lookup of keywords from the file-med dictionary in each index type.
It searches increasing numbers of keywords, like in *Figure 10* from the paper, clearing the cache and starting up the index before each lookup.
The lookup time for each index and each number of keywords is logged by the script in the following csv file: *lookup.log*.
The script creates *lookup.log* in a CSV format, where the interesting values are *index_type*, *keywords_num* and *total_lookup_time*. *Index_type* is `naive` for Naive, and `dedup` for IDEA, *keywords_num* represents the number of keywords, and the *total_lookup_time* is in seconds. All the results are for the file-med dictionary.



##### Figure 11
[estimated runtime for LNX-198: <5 minutes]

The script `scripts/figure_11.sh` performs a lookup of 128 keywords from each dictionary in each index type, like in *Figure 11* from the paper, clearing the cache and starting up the index before each lookup.
The lookup time for each index and each number of keywords is logged by the script in the following csv file: *lookup.log*.
The script runs lookups for the dictionaries one after the other: file-low, file-med, file-high, chunk-low, chunk-med, chunk-high - for each dictionary, first runs the Naive version and then the IDEA version. Therefore there are 12 result rows in the CSV file.

**When the system is configured with offsets enabled, the `figure_11.sh` script can be used to reproduce Figure 15**

##### Figure 16
[estimated runtime for LNX-198: <1 minutes]

The script `scripts/figure_16.sh` performs a lookup of a single keyword from the file-med and then the chunk-med dictionaries in each index type, like in *Figures 9+16* from the paper.
The lookup time for each index and each dictionary is logged by the script in the following csv file: `lookup.log`.
There are 4 single keywords averaged - `1a`, `1b`, `1c` and `1d`. For each dictionary they are looked up in that order, first for the Naive version and then for the IDEA version. Therefore the CSV file contains 16 result rows.

# DATASETS
Dataset resources: [LINUX KERNEL MIRROR](https://mirrors.kernel.org/), [ENWIKI DUMPS](https://dumps.wikimedia.org/enwiki/) and the old Wikipedia mirrors on the [INTERNET ARCHIVE](https://archive.org/).

For more information on how the datasets were built and how to recreate them, check the `dataset_details` directory.


# KNOWN ISSUES
##### `locale::facet::_S_create_c_locale name not valid`
This error may be introduced on computers with a locale which Lucene cannot recognize.
It can be solved by running the following command:
`export LC_ALL="en_US.UTF-8"`
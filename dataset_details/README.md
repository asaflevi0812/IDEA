# DATASETS
In order to retrieve the Linux and Wikipedia datasets, you may use the following resources:

[LINUX KERNEL MIRROR](https://mirrors.kernel.org/)

We used the versions between 2.0 and 5.9 (inclusive).

LNX-198: contains all the minor versions in this range.

LNX-409: contains every 10th patch version in this range (.0, .10, .20 and so on).

LNX-662: contains every 5th patch version in this range (.0, .5, .10, .15 and so on).

[ENWIKI DUMPS](https://dumps.wikimedia.org/enwiki/)

We used old versions of the same enwiki dump from the wikipedia dumps.
The versions we used are between January 2017 and March 2018.
Each month there are two backups, in the First and Twentieth of each month.
You may access these versions using the internet archive:
[01/01/2017](https://archive.org/details/enwiki-20170101),
[20/01/2017](https://archive.org/details/enwiki-20170120),
[01/02/2017](https://archive.org/details/enwiki-20170201),
[20/02/2017](https://archive.org/details/enwiki-20170220),
etc. (link format: https://archive.org/details/enwiki-YYYYMMDD).

The relevant file to retrieve is `enwiki-YYYYMMDD-pages-articles.xml.bz2`.

We split this file into smaller, 100MB files, after decompression.

WIKI-24 was created by using all available versions between January 2017 and March 2018. 

WIKI-12, WIKI-8 and WIKI-4 were created by a conseuctive subset of the WIKI-24 version. 

There were also WIKI-24 and WIKI-12 versions with files of size 1MB.

Each DATASET.txt file in this directory sepcifies the exact versions used in that dataset.
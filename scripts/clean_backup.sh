#!/bin/bash
while [[ $(findmnt working_directories) != "" ]]; do sudo umount working_directories; done
rm -rf working_directories/backup_directory/*
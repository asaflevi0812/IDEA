#!/bin/bash
while [[ $(findmnt working_directories) != "" ]]; do sudo umount working_directories; done
rm -rf working_directories/index_directory/* working_directories/chunk_to_file_directory/*
mkdir -p working_directories/index_directory working_directories/chunk_to_file_directory
mkdir -p working_directories/backup_directory
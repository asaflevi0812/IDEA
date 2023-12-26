#!/bin/bash
sudo cp src/lucene++_bin/liblucene++*.so* /usr/lib/ && \
rm -rf build && mkdir build && \
bash -c "$(python3 create_build_command.py)" && \
sudo cp build/destor /usr/local/bin/destor
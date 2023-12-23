#!/bin/bash
cp scripts/index_title.txt index.log
scripts/clear_cache.sh
destor -an
scripts/clear_cache.sh
destor -aq
#!/bin/bash
cp scripts/lookup_title.txt lookup.log
scripts/clear_cache.sh
destor -m -f"keywords/linux/16/file-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/16/file-med.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/32/file-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/32/file-med.txt"
scripts/clear_cache.sh  
destor -m -f"keywords/linux/64/file-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/64/file-med.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/file-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/file-med.txt"

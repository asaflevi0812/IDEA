#!/bin/bash
cp scripts/lookup_title.txt lookup.log
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/file-low.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/file-low.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/file-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/file-med.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/file-high.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/file-high.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/chunk-low.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/chunk-low.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/chunk-med.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/chunk-med.txt"
scripts/clear_cache.sh
destor -m -f"keywords/linux/128/chunk-high.txt"
scripts/clear_cache.sh
destor -l -f"keywords/linux/128/chunk-high.txt"

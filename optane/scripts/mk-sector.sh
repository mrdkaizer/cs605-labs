#!/bin/bash

umount /mnt/pmem3
sudo ndctl create-namespace --force --reconfig=namespace3.0 --mode=sector
mkfs -t xfs -f /dev/pmem3s
mount /dev/pmem3s /mnt/pmem3

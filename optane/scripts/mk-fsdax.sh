#!/bin/bash

umount /mnt/pmem3
sudo ndctl create-namespace --force --reconfig=namespace3.0 --mode=fsdax
mkfs -t xfs -f /dev/pmem3
mount -o dax /dev/pmem3 /mnt/pmem3

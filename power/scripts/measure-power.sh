#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <TIME>"
  exit
fi

TIME=$1

PERF_OUT=$(sudo perf stat -x -a -e power/energy-pkg/ sleep ${TIME} 2>&1)
PKG_ENERGY=$(echo ${PERF_OUT} | awk -F- '{print $1}')
PKG_POWER=$(echo "scale=2; $PKG_ENERGY/${TIME}" | bc -l)
echo "energy-pkg   (J)  ${PKG_ENERGY}"
echo "energy-power (W)  ${PKG_POWER}"

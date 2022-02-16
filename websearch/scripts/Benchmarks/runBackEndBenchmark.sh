#!/bin/bash

for thread in 1 2 4 8 16 20
do

    echo "INDEX_SERVER=\"node0\"
INDEX_THREADS=$thread

INDEXES_COUNT=32

FRONTEND_SERVER=\"node0\"
FRONTEND_INSTANCES_PER_SERVER=20

WEBSEARCH_HOME_DIR=\"/local/websearch\"" > ../scripts/common.config

    ../index.sh stop
    ../index.sh config
    ../index.sh start
    echo 'Press enter to continue, current backend threads:' $thread
    read line

done
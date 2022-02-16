#!/bin/bash

for thread in 1 2 4 8 16 32
do

    echo "INDEX_SERVER=\"node0\"
INDEX_THREADS=1

INDEXES_COUNT=$thread

FRONTEND_SERVER=\"node0\"
FRONTEND_INSTANCES_PER_SERVER=1

WEBSEARCH_HOME_DIR=\"/local/websearch\"" > ./common.config

    ./index.sh stop
    ./index.sh config
    ./index.sh start
    echo 'Press enter to continue, current partition threads:' $thread
    read line

done
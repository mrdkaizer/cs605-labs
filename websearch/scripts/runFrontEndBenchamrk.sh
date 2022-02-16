#!/bin/bash

for thread in 1 2 4 8 16 20
do

    echo "INDEX_SERVER=\"node0\"
INDEX_THREADS=20

INDEXES_COUNT=32

FRONTEND_SERVER=\"node0\"
FRONTEND_INSTANCES_PER_SERVER=$thread

WEBSEARCH_HOME_DIR=\"/local/websearch\"" > ./common.config

    ../frontend.sh stop
    ../frontend.sh config
    ../frontend.sh start
    echo 'Press enter to continue, current front end threads:' $thread
    read line
done
#!/bin/bash
EXPIREMENT_SIZE=10

> results
for thread in 1 2 4 8 16 20
do
    for i in $(seq 1 $EXPIREMENT_SIZE)
    do
	    ../client node0 8080 /local/websearch/ISPASS_PAPER_QUERIES_100K 1000 $thread onlyHits.jsp 1 1 /tmp/out 0 2> /dev/null >> results
        echo "---" >> results
    done
done
#!/bin/bash

EXPIREMENT_SIZE=10

> aggregated_ops_per_second
var="$(cat results | grep "ops/sec"|cut -d ' ' -f2-)"

c=0
result=0

for i in $var
do
    c=$((c+1))
    result=$(echo "$result + $i" | bc)

    if ! ((c % $EXPIREMENT_SIZE));
    then
        result=$(echo "$result / $EXPIREMENT_SIZE"| bc -l)
        echo $result >> aggregated_ops_per_second
        result=0
    fi

done

> aggregated_average
var="$(cat results | grep "average"|cut -d ' ' -f2-)"

c=0
result=0

for i in $var
do
    c=$((c+1))
    result=$(echo "$result + $i" | bc)

    if ! ((c % $EXPIREMENT_SIZE));
    then
        result=$(echo "$result / $EXPIREMENT_SIZE"| bc -l)
        echo $result >> aggregated_average
        result=0
    fi

done

> aggregated_90th
var="$(cat results | grep "90th"|cut -d ' ' -f2-)"

c=0
result=0

for i in $var
do
    c=$((c+1))
    result=$(echo "$result + $i" | bc)

    if ! ((c % $EXPIREMENT_SIZE));
    then
        result=$(echo "$result / $EXPIREMENT_SIZE"| bc -l)
        echo $result >> aggregated_90th
        result=0
    fi

done

> aggregated_99th
var="$(cat results | grep "99th"|cut -d ' ' -f2-)"

c=0
result=0

for i in $var
do
    c=$((c+1))
    result=$(echo "$result + $i" | bc)

    if ! ((c % $EXPIREMENT_SIZE));
    then
        result=$(echo "$result / $EXPIREMENT_SIZE"| bc -l)
        echo $result >> aggregated_99th
        result=0
    fi

done
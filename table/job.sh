#!/bin/bash

if [ ! -s "data/res_$1_$2.txt" ]
then
	./mmcts_$1 $RANDOM > data/res_$1_$2.txt
fi
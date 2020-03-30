#!/bin/bash

mkdir -p data

# initialize the job pool
thread_count=4;
if [ $# -eq 1 ]; then
	thread_count=$1 
fi
. job_pool.sh
job_pool_init $thread_count 0

# run jobs
p="CHAOS"
cp=1
dims=(8 10 12)
lens=(6 7 8)
deps=(100000 100000 100000)

for a in ${!dims[@]}; do
	w=${dims[$a]}
	m=${lens[$a]}
	d=${deps[$a]}
	jobstr="${cp}_${p}_${w}_${m}_${d}"
	if [ -s "data/res_${jobstr}.txt" ]
	then
		echo "Skipping job ${jobstr}"
	else
		g++ -std=c++17 mmcts.cc -o "mmcts_${jobstr}" -Dboard_w=${w} -Dboard_m=${m} -Dboard_d=${d} -DCAN_PASS=${cp} -DPASS_PLAYER=${p}
		for j in {1..100}; do
			job_pool_run ./job.sh "${jobstr}" $j
		done
		# sync
		job_pool_wait
		# merge
		cat data/res_${jobstr}_*.txt > data/res_${jobstr}.txt
		rm mmcts_${jobstr}
		rm data/res_${jobstr}_*.txt
		echo "Job ${jobstr} done!"
	fi
done

# don't forget to shut down the job pool
job_pool_shutdown
echo "Finished"
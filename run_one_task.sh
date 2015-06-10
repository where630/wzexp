#!/bin/bash

#para 1: task name
#para 2: frequency
#para 3: coreid
#para 4: utilization
run_one_task()
{
    cmd="./"${1}" &"
    $cmd > taskinfo.txt &
    pid=$!
    sudo cpufreq-set -c $3 -f $2
    taskset -cp $3 $pid
    cpulimit -p $pid -l $4
}

task=`tail -n 1 running_tasks.txt`
arr=($task)
#echo ${arr[3]}
run_one_task ${arr[2]} ${arr[3]} ${arr[4]} ${arr[5]}
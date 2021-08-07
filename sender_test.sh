#!/bin/bash

./read -m10 < Philosophen.txt &  # start sender in background
 
PID_SENDER=$! # store process ID of newly created sender process

kill -STOP ${PID_SENDER} # send SIGSTOP to sender which is blocking in sem_wait() operation
kill -CONT ${PID_SENDER}  # send SIGCONT to sender which is blocking in sem_wait() operation

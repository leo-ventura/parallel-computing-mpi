#!/bin/bash

for num_procs in 1 2 4 8 16 32 64
do
    echo "num_procs = $num_procs"
    if [ ! -f ~/masb/$@_$num_procs ]; then
        touch ~/masb/$@_$num_procs
        echo "num_procs = $num_procs" >> ~/masb/$@_$num_procs
    fi
    if [ -f ~/masb/$@_$num_procs ]; then
        REPEAT=$( expr 31 - $( wc -l < ~/masb/$@_$num_procs ) )
        echo Exec plan: $REPEAT
        for (( it=0; it<$REPEAT; it++ ))
        do

            NUM=`~/submission.sh -np $num_procs /work/grupo_e/$@`
            NUM=${NUM::-24}
            echo "Item $NUM"
            while [ ! -f ~/STDIN.o$NUM ]; do sleep 1; done

            cat ~/STDIN.o$NUM >> ~/masb/$@_$num_procs

        done
    fi
done
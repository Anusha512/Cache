#!/bin/bash
function run()
{
    echo -n "Running on test 7...  verdict:"
    ./sim_cache 16 1024 8 0 8192 4 traces/go_trace.txt > valid7.txt
    #diff ../results/DebugRun1.txt ../validation/ValidationRun1.txt
    if [ "${?}" == "0" ]
        then
            echo "OK"
    fi
}

make clean
make

mkdir -p ../results

run
echo "All done."
exit 0


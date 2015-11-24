#!/bin/bash
function run()
{
    echo -n "Running on test 7...  verdict:"
    ./sim_cache 16 1024 8 0 8192 4 ../traces/go_trace.txt > DebugRun7.txt
    diff -iw DebugRun7.txt ../validation/Validation7_PartB.txt
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


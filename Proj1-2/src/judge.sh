#!/bin/bash
function run()
{
    echo -n "Running on test 6...  verdict:"
    ./sim_cache 32 2048 4 0 4096 8     ../traces/gcc_trace.txt  > ../results/DebugRun6.txt
    diff -iw ../results/DebugRun6.txt  ../validation/Validation6_PartB.txt
    if [ "${?}" == "0" ]
        then
            echo "OK"
    fi

    echo -n "Running on test 7...  verdict:"
    ./sim_cache 16 1024 8 0 8192 4     ../traces/go_trace.txt   > ../results/DebugRun7.txt
    diff -iw ../results/DebugRun7.txt  ../validation/Validation7_PartB.txt
    if [ "${?}" == "0" ]
        then
            echo "OK"
    fi

    echo -n "Running on test 8...  verdict:"
    ./sim_cache 32 1024 8 256 0 0      ../traces/perl_trace.txt > ../results/DebugRun8.txt
    diff -iw ../results/DebugRun8.txt  ../validation/Validation8_PartB.txt
    if [ "${?}" == "0" ]
        then
            echo "OK"
    fi

    echo -n "Running on test 9...  verdict:"
    ./sim_cache 128 1024 2 1024 4096 4 ../traces/gcc_trace.txt  > ../results/DebugRun9.txt
    diff -iw ../results/DebugRun9.txt  ../validation/Validation9_PartB.txt
    if [ "${?}" == "0" ]
        then
            echo "OK"
    fi

    echo -n "Running on test 10..  verdict:"
    ./sim_cache 64 8192 2 1024 16384 4 ../traces/perl_trace.txt > ../results/DebugRun10.txt
    diff -iw ../results/DebugRun10.txt ../validation/Validation10_PartB.txt
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


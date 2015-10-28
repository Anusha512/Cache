function run_tests()
{
    echo "Running..."
    ./sim_cache  16 16384 1 0 0 ../traces/gcc_trace.txt  > ../results/DebugRun1.txt
    echo "Running on test 1..."
    ./sim_cache 128  2048 8 0 1 ../traces/go_trace.txt   > ../results/DebugRun2.txt
    echo "Running on test 2..."
    ./sim_cache  32  4096 4 0 1 ../traces/perl_trace.txt > ../results/DebugRun3.txt
    echo "Running on test 3..."
    ./sim_cache  64  8192 2 1 0 ../traces/gcc_trace.txt  > ../results/DebugRun4.txt
    echo "Running on test 4..."
    ./sim_cache  32  1024 4 1 1 ../traces/go_trace.txt   > ../results/DebugRun5.txt
    echo "Running on test 5..."
    #echo "Running all tests.. stop"
}

function diff_tests()
{
    echo "Comparing..."
    diff ../results/DebugRun1.txt ../validation/ValidationRun1.txt
    if [ "${?}" == "0" ]
        then
            echo "Accepted"
    fi
    diff ../results/DebugRun2.txt ../validation/ValidationRun2.txt
    if [ "${?}" == "0" ]
        then
            echo "Accepted"
    fi
    diff ../results/DebugRun3.txt ../validation/ValidationRun3.txt
    if [ "${?}" == "0" ]
        then
            echo "Accepted"
    fi
    diff ../results/DebugRun4.txt ../validation/ValidationRun4.txt
    if [ "${?}" == "0" ]
        then
            echo "Accepted"
    fi
    diff ../results/DebugRun5.txt ../validation/ValidationRun5.txt
    if [ "${?}" == "0" ]
        then
            echo "Accepted"
    fi
}

make clean
make

mkdir -p ../results

run_tests
diff_tests
echo "All works' done."
exit 0


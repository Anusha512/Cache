#Hint

If you got a compile error in Ubuntu, just try

`sudo apt-get install g++-4.8-multilib`

`sudo apt-get install libc6-dev-i386`

`sudo apt-get install build-essential`

and run `make clean && make` again.

#Validation

```
./sim_cache 32 2048 4 0 4096 8		gcc_trace.txt  > DebugRun6.txt
diff 	DebugRun6.txt  	Validation6_PartB.txt
    
./sim_cache 16 1024 8 0 8192 4     go_trace.txt   > DebugRun7.txt
diff 	DebugRun7.txt  	Validation7_PartB.txt
    
./sim_cache 32 1024 8 256 0 0      perl_trace.txt > DebugRun8.txt
diff 	DebugRun8.txt  	Validation8_PartB.txt
    
./sim_cache 128 1024 2 1024 4096 4 gcc_trace.txt  > DebugRun9.txt
diff 	DebugRun9.txt  	Validation9_PartB.txt
    
./sim_cache 64 8192 2 1024 16384 4 perl_trace.txt > DebugRun10.txt
diff 	DebugRun10.txt 	Validation10_PartB.txt
    
```
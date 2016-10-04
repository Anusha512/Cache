#Build

`make clean && make`

If you get a compile error in Ubuntu, just try

`sudo apt-get install g++-4.8-multilib`

`sudo apt-get install libc6-dev-i386`

`sudo apt-get install build-essential`

and run `make clean && make` again.

#Validation

##In Proj1-1
```
./sim_cache  16 16384 1 0 0 		gcc_trace.txt  > DebugRun1.txt
diff 	DebugRun1.txt 	ValidationRun1.txt
    
./sim_cache 128  2048 8 0 1 		go_trace.txt   > DebugRun2.txt
diff 	DebugRun2.txt 	ValidationRun2.txt
    
./sim_cache  32  4096 4 0 1 		perl_trace.txt > DebugRun3.txt
diff 	DebugRun3.txt 	ValidationRun3.txt
    
./sim_cache  64  8192 2 1 0 		gcc_trace.txt  > DebugRun4.txt
diff 	DebugRun4.txt 	ValidationRun4.txt
    
./sim_cache  32  1024 4 1 1 		go_trace.txt   > DebugRun5.txt
diff 	DebugRun5.txt 	ValidationRun5.txt
    
```

##In Proj1-2
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

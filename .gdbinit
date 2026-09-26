set architecture i386
set pagination off
target remote localhost:1234
file build/bin/kerneldbg.bin
break main

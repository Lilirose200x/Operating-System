mysh: shell.c interpreter.c shellmemory.c pcb.c kernel.c cpu.c 
	
	gcc -D FRAME_MEM_LENGTH=$(framesize) -D VAR_MEM_LENGTH=$(varmemsize) -c shellmemory.c shell.c interpreter.c shellmemory.c pcb.c kernel.c cpu.c 
	gcc -o mysh shell.o interpreter.o shellmemory.o pcb.o cpu.o kernel.o


clean: 
	rm mysh; rm *.o

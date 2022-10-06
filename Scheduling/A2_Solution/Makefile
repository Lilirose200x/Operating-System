mysh: shell.c interpreter.c shellmemory.c
	gcc -c shell.c interpreter.c shellmemory.c  pcb.c
	gcc -o mysh shell.o interpreter.o shellmemory.o  pcb.o

clean: 
	rm mysh; rm *.o

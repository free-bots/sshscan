default: program

sshscan.o: sshscan.c
	gcc -c sshscan.c -o sshscan.o

program: sshscan.o
	gcc -o sshscan sshscan.c

clean:
	-rm -f sshscan.o
	-rm -f sshscan
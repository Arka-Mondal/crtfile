CCFLAGS = -Wall -Wextra

crtfile: crtfile.o
	gcc -o crtfile crtfile.o

crtfile.o: crtfile.c
	gcc -c crtfile.c $(CCFLAGS)

.PHONY : clean
clean:
	rm -vf crtfile crtfile.o

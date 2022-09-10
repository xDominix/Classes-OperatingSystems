static:
	@echo "Creating static library..."
	make clean	
	gcc -Wall -c mylib.c
	ar crs libmylib.a mylib.o
	gcc -Wall main.c -o main -L. -lmylib

shared:
	@echo "Creating shared library..."
	make clean
	gcc -Wall -fPIC -c mylib.c
	gcc -Wall -shared -fPIC -o libmylib.so mylib.o
	gcc -Wall main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

clean:
	@echo "Cleaning up..."
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

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

dynamic:
	@echo "Creating dynamic library..."
	make clean
	gcc -Wall -fPIC -c mylib.c -o mylib.o
	gcc -Wall -Wl,-rpath=. -fPIC -shared -o libmylib.so mylib.o
	gcc -Wall -fPIC -L. -o main main_dynamic.c -ldl -D DYNAMIC

clean:
	@echo "Cleaning up..."
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

test:
	@echo "Small test..."
	./main create_table 1 wc_files 1 file00.txt remove_block 0
	./main create_table 2 wc_files 2 file01.txt file02.txt remove_block 1

	@echo "Medium test..."
	./main create_table 1 wc_files 1 file10.txt remove_block 0
	./main create_table 2 wc_files 2 file11.txt file12.txt remove_block 1 remove_block 0

	@echo "Large test..."
	./main create_table 2 wc_files 2 file20.txt file21.txt remove_block 0 remove_block 1
	./main create_table 3 wc_files 3 file22.txt file23.txt file24.txt remove_block 0 remove_block 1
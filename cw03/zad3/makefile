test:
	rm -f main
	gcc -Wall -o main main.c
	./main testdir "lamp" 3 #should found

test1:
	rm -f main
	gcc -Wall -o main main.c
	./main testdir "depth" 3 #should not found

test2:
	rm -f main
	gcc -Wall -o main main.c
	./main testdir "depth" 4 #should found

test3:
	rm -f main
	gcc -Wall -o main main.c
	./main testdir "qwerty" 0 #should found

clean:
	rm -f main

make kill:
	gcc -Wall -o sender sender.c && gcc -Wall -o catcher catcher.c
	./catcher KILL ###NOW TRY--->./sender <pid> 5 KILL###

make queue:
	gcc -Wall -o sender sender.c && gcc -Wall -o catcher catcher.c
	./catcher SIGQUEUE ###NOW TRY--->./sender <pid> 5 SIGQUEUE###

make rt:
	gcc -Wall -o sender sender.c && gcc -Wall -o catcher catcher.c
	./catcher SIGRT ###NOW TRY--->./sender <pid> 5 SIGRT###

make clean:
	rm -f catcher
	rm -f sender


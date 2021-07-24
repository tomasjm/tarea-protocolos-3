all:
	g++ -Wall -c anillo.cpp -lwiringPi
	g++ -Wall -c ethernet/ethernet.cpp
	g++ -Wall -c protocol/protocol.cpp
	g++ -Wall -c slip/slip.cpp
	g++ -Wall -c menu/menu.cpp
	g++ -Wall -c helpers/helpers.cpp
	g++ -Wall -c ring/ring.cpp
	g++ -Wall -o anillo anillo.o ethernet.o protocol.o slip.o menu.o helpers.o ring.o -lwiringPi
test:
	g++ -Wall -c test.cpp -lwiringPi
	g++ -Wall -c ethernet/ethernet.cpp
	g++ -Wall -c protocol/protocol.cpp
	g++ -Wall -c slip/slip.cpp
	g++ -Wall -c menu/menu.cpp
	g++ -Wall -c helpers/helpers.cpp
	g++ -Wall -c ring/ring.cpp
	g++ -Wall -o test test.o ethernet.o protocol.o slip.o menu.o helpers.o ring.o -lwiringPi
run1:
	sudo ./anillo 11:27:bb:44:b1:ca 0 2 3 9 8
run2:
	sudo ./anillo xf:5b:2a:1a:b1:ff 23 24 25 22 21
run3:
	sudo ./anillo b8:27:eb:15:b1:ca 1 4 5 29 28
runtest:
	./test b8:27:eb:15:b1:ca 1 4 5 29 28
clean:
	rm *.o

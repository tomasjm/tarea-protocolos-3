all:
	g++ -Wall -c punto.cpp -lwiringPi
	g++ -Wall -c protocol/protocol.cpp
	g++ -Wall -c helpers/helpers.cpp
	g++ -Wall -c menu/menu.cpp
	g++ -Wall -c ethernet/ethernet.cpp
	g++ -Wall -c slip/slip.cpp
	g++ -Wall -o punto punto.o ethernet.o menu.o protocol.o slip.o helpers.o -lwiringPi
test:
	g++ -Wall -c anillo.cpp -lwiringPi
	g++ -Wall -c ethernet/ethernet.cpp
	g++ -Wall -c protocol/protocol.cpp
	g++ -Wall -c slip/slip.cpp
	g++ -Wall -c menu/menu.cpp
	g++ -Wall -c helpers/helpers.cpp
	g++ -Wall -o anillo anillo.o ethernet.o protocol.o slip.o menu.o helpers.o -lwiringPi
clean:
	rm *.o

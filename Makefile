all : winsuport.o winsuport2.o memoria.o semafor.o tron4

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

winsuport2.o: winsuport2.c winsuport2.h
	gcc -c winsuport2.c -o winsuport2.o		

memoria.o: memoria.c memoria.h
	gcc -c memoria.c -o memoria.o

semafor.o: semafor.c semafor.h
	gcc -c semafor.c -o semafor.o

tron0 : tron0.c winsuport.o winsuport.h
	gcc -Wall tron0.c winsuport.o -o tron0 -lcurses

tron1 : tron1.c winsuport.o winsuport.h
	gcc -Wall tron1.c winsuport.o -o tron1 -lcurses

tron3 : tron3.c oponent3.c tron.h 
	gcc tron3.c winsuport2.o memoria.o semafor.o -o tron3 -lcurses -lpthread
	gcc oponent3.c winsuport2.o memoria.o semafor.o -o oponent3 -lcurses

tron4 : tron4.c oponent3.c tron.h 
	gcc tron4.c winsuport2.o memoria.o semafor.o -o tron4 -lcurses -lpthread
	gcc oponent3.c winsuport2.o memoria.o semafor.o -o oponent3 -lcurses
	
clean: 
	rm winsuport.o winsuport2.o memoria.o semafor.o tron0 tron1 tron3

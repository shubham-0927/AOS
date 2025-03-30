output: main.o cd.o echo.o ls.o search.o proid.o handletab.o pipeline.o redirection.o
	g++ main.o cd.o echo.o ls.o search.o proid.o handletab.o pipeline.o redirection.o -o output

main.o: main.cpp header.h
	g++ -c main.cpp

redirection.o: redirection.cpp header.h
	g++ -c redirection.cpp

pipeline.o: pipeline.cpp header.h
	g++ -c pipeline.cpp

handletab.o: handletab.cpp header.h
	g++ -c handletab.cpp

search.o: search.cpp header.h
	g++ -c search.cpp

ls.o: ls.cpp header.h
	g++ -c ls.cpp

cd.o: cd.cpp header.h
	g++ -c cd.cpp

echo.o: echo.cpp header.h
	g++ -c echo.cpp

proid.o: proid.cpp header.h
	g++ -c proid.cpp

clean:
	rm *.o *.txt

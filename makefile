server: threadpool.o server.o processinfo.o diskinfo.o osinfo.o
	g++ -o server threadpool.o server.o processinfo.o diskinfo.o osinfo.o -pthread -w -fpermissive
	rm threadpool.o server.o processinfo.o diskinfo.o osinfo.o osinfo.h.gch diskinfo.h.gch processinfo.h.gch threadpool.h.gch
processinfo.o: processinfo.h processinfo.cpp
	g++ -c processinfo.h processinfo.cpp
diskinfo.o: diskinfo.h diskinfo.cpp
	g++ -c diskinfo.h diskinfo.cpp
osinfo.o: osinfo.h osinfo.cpp
	g++ -c osinfo.cpp osinfo.h
threadpool.o: threadpool.h threadpool.cpp
	g++ -c threadpool.cpp threadpool.h -w -fpermissive -pthread
server.o: server.cpp
	g++ -c server.cpp
.PHONY : clean
clean :
	rm server
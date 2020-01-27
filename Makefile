
dump.exe: dump.cc
	g++ dump.cc -o dump.exe -std=c++17

order.txt chaos.txt: dump.exe
	./dump.exe
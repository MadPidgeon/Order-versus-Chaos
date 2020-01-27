
dump.exe: dump.cc
	g++ dump.cc -o dump.exe -std=c++17

order.txt chaos.txt: dump.exe
	./dump.exe

filter.exe: filter.cc
	g++ filter.cc -o filter.exe -std=c++17

order.json: filter.exe order.txt
	./filter.exe

tree.dot: order.json decision_tree.py
	python decision_tree.py

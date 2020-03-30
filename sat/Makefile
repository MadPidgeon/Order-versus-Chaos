visual_solution.txt: solution.txt visual
	./visual < solution.txt > visual_solution.txt

visual: visual.cc
	g++ -std=c++17 visual.cc -o visual

sat_generator: sat_generator.cc
	g++ -std=c++17 sat_generator.cc -o sat_generator

sat.cnf: sat_generator
	./sat_generator > sat.cnf

solution.txt: sat.cnf
	-picosat sat.cnf > solution.txt

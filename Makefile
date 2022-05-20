COMPILATION_UNITS = main.cpp b_plus_tree.cpp tabela.cpp 
INCLUDE = ./include/
HEADERS = b_plus_tree.hpp  consts.hpp  csv.hpp  file.hpp  internal_node.hpp  key.hpp  leaf_node.hpp  misc.hpp  node.hpp  operador.hpp  tabela.hpp
HEADERS := $(addprefix $(INCLUDE), $(HEADERS))
CSVS = pais.csv uva.csv vinho.csv

main: $(COMPILATION_UNITS) $(HEADERS) $(CSVS)
	g++ -std=c++2a $(COMPILATION_UNITS) -o main

.PHONY: run

run: main
	./main
all:
	g++ -std=c++2a -O3 test.cpp -o graph_test -lfmt -I include/

debug:
	g++ -std=c++2a -g3 test.cpp -o graph_debug -lfmt -DGRAPH_ENABLE_DBG=1 -I include/

example:
	g++ -std=c++2a -O3 example.cpp -o example -I include/ && ./example

test: all 
	./graph_test

clean:
	rm -f graph_test
	rm -f graph_debug
	rm -f example
	rm -rf *.dSYM


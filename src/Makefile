all: histogram histo_private histo_lockfree

histogram: histogram.cpp ppmb_io.a
	gcc -O3 $^ -o $@ -lm -lrt

histo_private: histo_private.cpp ppmb_io.a
	g++ -std=c++0x -pthread -O3 $^ -o $@ -lm -lrt

histo_lockfree: histo_lockfree.cpp ppmb_io.a
	g++ -std=c++0x -pthread -O3 $^ -o $@ -lm -lrt

ppmb_io.a: ppmb_io.o
	ar rs $@ $<

.phony: clean

clean:
	rm -f ppmb_io.a ppmb_io.o histogram


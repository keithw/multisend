source = infer.cc process.cc sampledfunction.cc processforecaster.cc
objects = process.o sampledfunction.o processforecaster.o
executables = infer

CXX = g++
CXXFLAGS = -g -O3 -std=c++0x -ffast-math -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe -Wno-unused-parameter -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
LIBS = -lm -lrt

all: $(executables)

infer: infer.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

depend: $(source)
	$(CXX) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo
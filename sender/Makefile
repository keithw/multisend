source = send1way.cc socket.cc buffer-setpoint.cc hist.cc controlled-delay.cc rate-estimate.cc payload.cc history.cc delay-servo.cc saturatr.cc acker.cc saturateservo.cc cellsim.cc cellsim-tap.cc select.cc packetsocket.cc tapsocket.cc macaddr.cc
objects = socket.o hist.o rate-estimate.o payload.o history.o delay-servo.o acker.o saturateservo.o select.o packetsocket.o tapsocket.o macaddr.cc
executables = send1way buffer-setpoint controlled-delay saturatr cellsim cellsim-tap

CXX = g++
CXXFLAGS = -g -O3 -std=c++0x -ffast-math -pedantic -Wall -Wextra -Weffc++ -fno-default-inline -pipe -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
LIBS = -lm -lrt

all: $(executables)

send1way: send1way.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

buffer-setpoint: buffer-setpoint.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

controlled-delay: controlled-delay.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

saturatr: saturatr.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

cellsim: cellsim.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

cellsim-tap: cellsim-tap.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

depend: $(source)
	$(CXX) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo

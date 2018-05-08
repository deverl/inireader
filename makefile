
.DEFAULT : all

.PHONY : clean test

all : inireader

inireader : inireader.cpp makefile
	g++ -o inireader -g2 inireader.cpp


clean:
	rm -rf inireader *.o inireader.dSYM

test: inireader
	./inireader sample.ini  CLIENT   phone


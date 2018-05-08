
.DEFAULT : all

.PHONY : clean test

all : inireader

inireader : inireader.cpp makefile
	g++ -o inireader -g2 inireader.cpp


clean:
	rm -rf inireader *.o inireader.dSYM

test: inireader
	./inireader /Users/dstokes1/Downloads/intuit_creds_plain.ini TSHEETS_INTUIT_PROD CLIENT_SECRET


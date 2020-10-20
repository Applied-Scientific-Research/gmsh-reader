CXX=c++
LIBS=-lm

all : read_MSH_file.bin

%.bin : %.cpp
	${CXX} -o $@ $< ${LIBS}

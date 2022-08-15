OBJS:= iterating_linear.o FastRandom.o
CPPFLAGS:= -Ofast -march=native -g -flto
LIBS:= -ljpeg
TARGET:= ifs

all: clean jos
  
jos: ${OBJS}
	g++ ${CPPFLAGS} ${OBJS} -o ${TARGET} ${LIBS}

.cpp.o:
	g++ ${CPPFLAGS} -c $<

clean:
	rm -f ${OBJS} ${TARGET}

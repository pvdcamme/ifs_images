OBJS:= iterating_linear.o FastRandom.o Colorizer.o
CPPFLAGS:= -Ofast -march=native -g -flto
LIBS:= -ljpeg
TARGET:= ifs

  
all: ${OBJS}
	g++ ${CPPFLAGS} ${OBJS} -o ${TARGET} ${LIBS}

.cpp.o:
	g++ ${CPPFLAGS} -c $<

clean:
	rm -f ${OBJS} ${TARGET}

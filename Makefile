OBJS:= iterating_linear.o
CPPFLAGS:= -Ofast -march=native
LIBS:= -ljpeg
TARGET:= ifs

jos: ${OBJS}
	g++ ${CPPFLAGS} ${OBJS} -o ${TARGET} ${LIBS}

.cpp.o:
	g++ ${CPPFLAGS} -c $<

clean:
	rm ${OBJS} ${TARGET}

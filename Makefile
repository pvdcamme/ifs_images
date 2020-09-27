OBJS:= iterating_linear.o
CPPFLAGS:= -Ofast -march=native
TARGET:= ifs

jos: ${OBJS}
	g++ ${CPPFLAGS} ${OBJS} -o ${TARGET}

.cpp.o:
	g++ ${CPPFLAGS} -c $<

clean:
	rm ${OBJS} ${TARGET}

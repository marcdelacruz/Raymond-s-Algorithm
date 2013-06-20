#
# Marc DelaCruz
# CS 6378-5U1 AOS Project 2 Summer 2010 
# 


CFLAGS = -g -Wall

CC = g++
LIBS =  -lrt -lsocket -lnsl
INCLUDES =
OBJS = ClientSocket.o Process.o Fd.o ConnSocket.o ProcessArg.o TimedAction.o Message.o ServerSocket.o TimedActionHandler.o Utilities.o
SRCS = ClientSocket.cpp Process.cpp Fd.cpp ConnSocket.cpp ProcessArg.cpp TimedAction.cpp Message.cpp ServerSocket.cpp TimedActionHandler.cpp Utilities.cpp

HDRS = CallBack.h Process.h Fd.h ClientSocket.h ProcessArg.h TimedAction.h\
       ConnSocket.h ReadMessage.h TimedActionHandler.h Message.h Utilities.h\
       ServerSocket.h MessageDefs.h Singleton.h

all: ray

ray: Raymund.o ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o ray Raymund.o ${OBJS} ${LIBS}

Raymund.o: ${SRCS} Raymund.h ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c Raymund.cpp

Fd.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c Fd.cpp

ServerSocket.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c ServerSocket.cpp

ConnSocket.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c ConnSocket.cpp

ClientSocket.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c ClientSocket.cpp

Process.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c Process.cpp

ProcessArg.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c ProcessArg.cpp

TimedActionHandler.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c TimedActionHandler.cpp

TimedAction.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c TimedAction.cpp

Message.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c Message.cpp

Utilities.o: ${SRCS} ${HDRS}
	${CC} ${CFLAGS} ${INCLUDES} -c Utilities.cpp

clean:
	rm *.o ray

#
# The path to logger.h/cpp
LOGGERPATH = ..

SRCS = main.cpp $(LOGGERPATH)/logger.cpp
OBJS = $(SRCS:%.cpp=%.o)
LDFLAGS = -lboost_thread-mt

all: main

main: $(OBJS)
	g++ $(OBJS) -o logtest $(LDFLAGS)
	
$(OBJS):%.o:%.cpp
	g++ -I$(LOGGERPATH) -c $< -o $@ $(LDFLAGS)

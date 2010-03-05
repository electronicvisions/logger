# The path to logger.h/cpp
LOGGERPATH = ..

SRCS = main.cpp $(LOGGERPATH)/logger.cpp
OBJS = $(SRCS:%.cpp=%.o)

all: main

main: $(OBJS)
	g++ $(OBJS) -o logtest
	
$(OBJS):%.o:%.cpp
	g++ -I$(LOGGERPATH) -c $< -o $@

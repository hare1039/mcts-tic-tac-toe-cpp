TARGET	= mcts
OBJECT	= graph.dot graph.svg
CXX		= clang++
CXXFLAGS= -std=c++11 -O2
LIBS    =


all: main.cpp
	$(CXX) -o $(TARGET) $(CXXFLAGS) $(LIBS) main.cpp

.PHONY: clean
clean:
	rm $(TARGET) $(OBJECT)

.PHONY: svg
svg: all
	./mcts graph.dot &&\
	dot -Tsvg graph.dot -o graph.svg

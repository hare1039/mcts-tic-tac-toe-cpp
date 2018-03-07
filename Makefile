TARGET	    = mcts
OBJECT	    = graph.dot graph.svg
OBJECT_HTML = index.html
CXX		    = clang++
CXXFLAGS    = -std=c++11 -O2
LIBS        =


all: main.cpp
	$(CXX) -o $(TARGET) $(CXXFLAGS) $(LIBS) main.cpp

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECT) $(OBJECT_HTML)

.PHONY: html
html: all
	./mcts index.html

.PHONY: svg
svg: all
	./mcts graph.dot &&\
	dot -Tsvg graph.dot -o graph.svg

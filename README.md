# mcts-tic-tac-toe-cpp
monte carlo tree search on tic-tac-toe using cpp

# build
A compiler with c++11 support

e.g.
build phase `clang++ -o mcts -std=c++11 main.cpp`

# visualize
I add an export function that will generate `dot` graph representation. So you can draw the graph and analize it. 

e.g.
```
./mcts graph.dot
dot -Tsvg graph.dot -o graph.svg
```

#include <fstream>
#include "mcts.hpp"

int main(int argc, char *argv[])
{
    namespace mcts = monte_carlo_tree_search;
    mcts::Tree tree {mcts::Node::X};
    tree.set("./ooxx.txt");
    for (int i{0}; i < 500000; i++)
    {
        mcts::Node * node = tree.root.best_node();
        node->random_play();
    }
    if (argc == 2)
    {
	    std::fstream f(argv[1], std::ios::out);
	    tree.export_to(f);
    }
    else
	    tree.export_to();
    return 0;
}


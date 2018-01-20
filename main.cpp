#include <fstream>
#include "mcts.hpp"

int main(int argc, char *argv[])
{
    namespace mcts = monte_carlo_tree_search;
    mcts::Node::peice p = mcts::Node::O;
    mcts::Tree tree(p);
    tree.set();
    for(int i(0); i<100; i++)
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


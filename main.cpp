#include <fstream>
#include "mcts.hpp"

int main(int argc, char *argv[])
{
    namespace mcts = monte_carlo_tree_search;
    mcts::Tree tree {mcts::Node::O}; // current role
    tree.set("./ooxx.txt");
    for (int i{0}; i < 500000; i++)
    {
        mcts::Node * node = tree.root.best_node();

        // no more candidate node => simulation should stop
        if (node == nullptr)
	        break;
        node->random_play();
    }
    if (argc == 2)
    {
	    std::fstream f(argv[1], std::ios::out);
	    std::cout << "Generating graph to " << argv[1] << "\n";
	    tree.export_to(f);
    }
    else
	    tree.export_to();
    return 0;
}


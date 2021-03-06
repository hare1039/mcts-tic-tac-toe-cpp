#include <fstream>
#include "mcts.hpp"

int main(int argc, char *argv[])
{
    namespace mcts = monte_carlo_tree_search;
    mcts::Tree tree {mcts::Node::X}; // current role
    tree.set("./ooxx.txt");
    for (int i{0}; i < 500000; i++)
    {
	    auto next = tree.play();
	    if (next == mcts::OPERATION::STOP)
		    break;
    }
    if (argc == 2)
    {
	    std::fstream f(argv[1], std::ios::out);
	    std::cout << "Generating graph to " << argv[1] << "\n";
	    tree.export_to_html(f);
    }
    else
	    tree.export_to_dot();
    return 0;
}


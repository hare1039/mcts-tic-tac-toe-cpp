#include "mcts.hpp"

int main(int argc, char *argv[])
{
    namespace mcts = monte_carlo_tree_search;
    mcts::Node::peice p = mcts::Node::O;
    mcts::Tree tree(p);
    for(int i(0); i<1000; i++)
    {
        mcts::Node * node = tree.root.best_node(i);
        node->random_play();
    }
    return 0;
}


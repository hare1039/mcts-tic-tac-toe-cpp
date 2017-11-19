#include "mcts.hpp"

int main(int argc, char *argv[])
{
	namespace mcts = monte_carlo_tree_search;
	mcts::Tree tree;
	for(int i(0); i<1000; i++)
	{
		mcts::Node * node = tree.root.best_node(i);
		node->random_play(p);
	}
	return 0;
}

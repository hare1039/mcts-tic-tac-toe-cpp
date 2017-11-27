#ifndef __MCTS_HPP__
#define __MCTS_HPP__

#include <vector>
#include <random>
#include <functional>
#include <algorithm>

#include <iostream>
#include <sstream>
#include <limits>     // std::numeric_limits
#include <cmath>
#include <cstdint>    // uintptr_t

#define UNTIL(x) while(not(x))

namespace monte_carlo_tree_search
{
	constexpr int BOARD_SIZE = 3;
	struct Tree;
    class Node
    {
    public:
	    enum peice{ EMPTY, O, X };
	    Node(peice p): player(p)
        {
            for(int i(0); i < BOARD_SIZE; i++)
                for(int j(0); j < BOARD_SIZE; j++)
                    board[i][j] = EMPTY;
        }

        ~Node()
        {
            for(auto &n : child)
                delete n;
        }
	    friend class Tree;
	    
    public: // static zone
        static
        peice flip(peice p)
        {
            if(p == EMPTY)
                return EMPTY;
            
            return (p == O)? X: O;
        }

	    static
        peice winner(peice eval_board[BOARD_SIZE][BOARD_SIZE])
        {
            if((eval_board[0][0] == eval_board[0][1] && eval_board[0][1] == eval_board[0][2]) ||
               (eval_board[0][0] == eval_board[1][0] && eval_board[1][0] == eval_board[2][0]) )
                return eval_board[0][0];

            else if  ((eval_board[1][0] == eval_board[1][1] && eval_board[1][1] == eval_board[1][2]) ||
                      (eval_board[0][1] == eval_board[1][1] && eval_board[1][1] == eval_board[2][1]) ||
                      (eval_board[0][0] == eval_board[1][1] && eval_board[1][1] == eval_board[2][2]) ||
                      (eval_board[2][0] == eval_board[1][1] && eval_board[1][1] == eval_board[0][2]) )
                return eval_board[1][1];

            else if  ((eval_board[0][2] == eval_board[1][2] && eval_board[1][2] == eval_board[2][2]) ||
                      (eval_board[2][0] == eval_board[2][1] && eval_board[2][1] == eval_board[2][2]) )
                return eval_board[2][2];

            else
                return EMPTY;
        }
	    
	    static
	    std::tuple<int, int> gen_pos_from(peice brd[BOARD_SIZE][BOARD_SIZE])
		{
			int x(0), y(0);
			bool full = true;
			for(int i(0); i < BOARD_SIZE; i++)
				for(int j(0); j < BOARD_SIZE; j++)
					if(brd[i][j] == EMPTY)
					    {full = false; break;}
			if(not full)
			{
				do
				{
					x = random_in(0, BOARD_SIZE - 1);
					y = random_in(0, BOARD_SIZE - 1);
				} UNTIL (brd[x][y] == EMPTY);
				return std::make_tuple(x, y);
			}
			return std::make_tuple(0, 0);
		}
	    
	    static
	    int random_in(int lower_bound, int higher_bound /* inclusive */)
	    {
		    static std::mt19937 engine{std::random_device{}()};
		    std::uniform_int_distribution<int> distribution(lower_bound, higher_bound);
		    return distribution(engine);
	    }
	    
    public: // methods
        Node& set_board(peice src[BOARD_SIZE][BOARD_SIZE])
        {
            for(int i(0); i < BOARD_SIZE; i++)
                for(int j(0); j < BOARD_SIZE; j++)
                    board[i][j] = src[i][j];
            return *this;
        }

        Node& add_child(Node * n)
        {
            child.push_back(n);
            n->parent = this;
            return *this;
        }

        Node* best_node()
        {
            if(child.empty())
                return this;
            return (*std::max_element(child.begin(), child.end(), [](const Node *A, const Node *B){
                return A->UCT() < B->UCT();
		    }))->best_node();
        }

	    Node& generate_all_child()
		{
			bool next_step_wins = false;
			for(int i(0); i < BOARD_SIZE; i++)
                for(int j(0); j < BOARD_SIZE; j++)
	                if(board[i][j] == EMPTY)
	                {
		                Node * child = new Node(flip(this->player));
		                child->set_board(this->board);
		                child->board[i][j] = flip(this->player);
		                this->add_child(child);
		                if(winner(child->board) == flip(this->player))
			                next_step_wins = true;
	                }

			if(next_step_wins)
				this->child.erase(
					std::remove_if(this->child.begin(),
					               this->child.end(),
					               [this](Node *n){ return winner(n->board) != Node::flip(this->player)? delete n, true: false ;}),
					this->child.end());
			
			return *this;
		}

        Node& random_play()
        {
	        if(child.empty())
		        generate_all_child();
	        if(child.empty())
		        return *this; // draw; end game;
	        
	        Node* random_child = child.at(random_in(0, static_cast<int>(child.size()) - 1));
            peice eval_board[BOARD_SIZE][BOARD_SIZE];
            for(int i(0); i < BOARD_SIZE; i++)
                for(int j(0); j < BOARD_SIZE; j++)
                    eval_board[i][j] = random_child->board[i][j];
            peice p = flip(random_child->player);
            for(;;)
            {
	            int x, y;
                std::tie(x, y) = gen_pos_from(eval_board);
                if(x == 0)
                    break;
                p = flip(p);
                eval_board[x][y] = p;

                peice champ = winner(eval_board);
                if(champ != EMPTY)
                {
	                random_child->update_status(champ /* wins */);
	                break;
                }        
            }
            return *this;
        }

	    void update_status(peice wins)
        {
            if(wins == this->player)
                this->win++;
            this->total++;
            if(parent != nullptr)
                parent->update_status(wins);
        }
	    
    private:
        peice board[BOARD_SIZE][BOARD_SIZE];
        /* E == empty
         |   | 0 | 1 | 2 |
         | 0 |   |   |   |
         | 1 |   |   |   |
         | 2 |   |   |   |
         */
        int win = 0, total = 0;

        double UCT() const
        {
	        return (total == 0)? std::numeric_limits<double>::max():
		        static_cast<double>(win) / total + std::sqrt(2) * std::sqrt(std::log((parent)? parent->total: 1) / total);
        }

        Node * parent = nullptr;
	    peice  player;
        std::vector<Node *> child;
    };

    struct Tree
    {
	    Tree(Node::peice p): root(Node::flip(p)){}
        Node root;
	    
	    void export_to(std::ostream &o = std::cout)
		{
			o << "digraph monte_carlo_tree_search_result {\n";
			visit(&root, o);
			o << "}";
		}

	    void visit(Node *node, std::ostream &o)
		{
			o << "    " << uintptr_t(node) << "[label = \""
			  << "wins/total: " << node->win << "/" << node->total << ", " << (!node->parent? 0: node->UCT()) << "\n"
			  << node->board[0][0] << node->board[0][1] << node->board[0][2] << "\n"
			  << node->board[1][0] << node->board[1][1] << node->board[1][2] << "\n"
			  << node->board[2][0] << node->board[2][1] << node->board[2][2]
			  << "\"]";
			for(auto child: node->child)
			{
				o << "    " << uintptr_t(node)
				  << " -> " << uintptr_t(child) << ";\n";
				visit(child, o);
			}
		}
    };
}

#endif /* __MCTS_HPP__ */

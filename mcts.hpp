#ifndef __MCTS_HPP__
#define __MCTS_HPP__

#include <vector>
#include <array>
#include <list>
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
	namespace OPERATION
	{
		enum TYPE { SKIP = 0, STOP, CONTINUE };
	}
	
	struct Tree;
    class Node
    {
    public:
	    enum peice{ EMPTY, O, X };
	    
	    Node(peice p): player(p)
        {
	        get_leafnode_list().add(this);
        }

        ~Node()
        {
            for (auto &n : child)
                delete n;
            get_leafnode_list().remove(this);
        }
	    friend class Tree;

    public: // static zone
        static
        peice flip(peice p)
        {
            if (p == EMPTY)
                return EMPTY;

            return (p == O)? X: O;
        }

	    static
        peice winner(std::array<std::array<peice, BOARD_SIZE>, BOARD_SIZE> const &eval_board)
        {
            if ((eval_board[0][0] == eval_board[0][1] && eval_board[0][1] == eval_board[0][2]) ||
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
	    std::tuple<int, int> gen_pos_from(std::array<std::array<peice, BOARD_SIZE>, BOARD_SIZE> const & brd)
		{
			int x(0), y(0);
			bool full = true;
			for (int i(0); i < BOARD_SIZE; i++)
				for (int j(0); j < BOARD_SIZE; j++)
					if (brd[i][j] == EMPTY)
					    {full = false; break;}
			if (not full)
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

	    static
	    Node* best_node()
        {
	        return get_leafnode_list().max();
        }

    public: // methods
	    Node& set_board(std::array<std::array<peice, BOARD_SIZE>, BOARD_SIZE> &src)
        {
	        board = src;
            return *this;
        }

        Node& add_child(Node * n)
        {
            child.push_back(n);
            n->parent = this;
            return *this;
        }

	    Node& generate_all_child()
		{
			bool next_step_wins = false;
			for (int i(0); i < BOARD_SIZE; i++)
                for (int j(0); j < BOARD_SIZE; j++)
	                if (board[i][j] == EMPTY)
	                {
		                Node * child = new Node(flip(this->player));
		                child->set_board(this->board);
		                child->board[i][j] = flip(this->player);
		                this->add_child(child);
		                if (winner(child->board) == flip(this->player))
			                next_step_wins = true;
	                }
			if (next_step_wins)
				this->child.erase(
					std::remove_if(this->child.begin(),
					               this->child.end(),
					               [this](Node *n){ return winner(n->board) != Node::flip(this->player)? delete n, true: false ;}),
					this->child.end());

			return *this;
		}

	    OPERATION::TYPE expansion()
		{
			// remove myself out of leaf node list
		    get_leafnode_list().remove(this);
	        peice d = winner(this->board);

	        // this board already have winner. 
	        if (d != EMPTY)
	        {
		        this->update_status(d);
		        return OPERATION::SKIP;
	        }

	        // expansion
	        generate_all_child();

	        // No more children generated. => draw, but count as player 2 wins; end game;
	        if (child.empty())
	        {
		        this->update_status(peice::X);
		        return OPERATION::SKIP;
	        }

	        return OPERATION::CONTINUE;
		}

	    Node* random_child()
		{
		    return child.at(random_in(0, static_cast<int>(child.size()) - 1));
		}

        peice simulation()
        {
	        auto eval_board = this->board;
	        peice p = flip(this->player);
            for (;;)
            {
	            // get random position
	            int x, y;
                std::tie(x, y) = gen_pos_from(eval_board);

                // no more position, board full
                if (x == 0)                
	                return EMPTY;
                
                p = flip(p);
                eval_board[x][y] = p;

                peice champ = winner(eval_board);
                if (champ != EMPTY)
	                return champ;
            }
        }

	    void update_status(peice wins)
        {
            if (wins == this->player)
                this->win++;
            this->total++;
            if (parent != nullptr)
                parent->update_status(wins);
        }

    private:
	    std::array<std::array<peice, BOARD_SIZE>, BOARD_SIZE> board;
//	    peice board[BOARD_SIZE][BOARD_SIZE];
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

	    struct Leafnode_collector
	    {
		    std::list<Node *> data;
		    Node* max() { return data.front(); }

		    Leafnode_collector& add(Node * n)
			{
				return this->add(std::vector<Node *>{n});
			}

		    Leafnode_collector& add(std::vector<Node *> const &x)
			{
			    for (auto i: x)
				    data.push_back(i);
			    return *this;
			}

		    Leafnode_collector& remove(Node * n)
			{
			    auto target = find(data.begin(), data.end(), n);
			    if (target != data.end())
				    data.erase(target);

			    return *this;
			}

		    void show()
			{
				std::cout << "    size: " << data.size() << "\n";
				for (auto &n: data)
					std::cout << "    " << n << " => " << n->UCT() << "\n";
			}

	    	// ref from http://en.cppreference.com/w/cpp/algorithm/lower_bound
		    template<class ForwardIt, class T, class Compare=std::equal_to<Node *>>
		    ForwardIt find(ForwardIt first, ForwardIt last, const T& value, Compare comp={})
			{
				for (ForwardIt it = first; it != last; ++it)
					if (comp(*it, value))
						return it;
				return last;
			}
	    };

	    static
	    Leafnode_collector& get_leafnode_list()
		{
			static Leafnode_collector candidate;
			return candidate;
		}

        Node * parent = nullptr;
	    peice  player;
        std::vector<Node *> child;
    };


    struct Tree
    {
	    Tree(Node::peice p): root(p){}
        Node root;
	    void set (std::string path)
		{
			std::fstream src{path, std::ios::in};
			if (not src.is_open())
				std::cerr << path << " cannot open\n";

			for (int i{0}; i < 3; i++)
			{
				for (int j{0}; j < 3; j++)
				{
					char c = ' ';
				    src >> c;
					root.board[i][j] =
						(c == 'O' || c == 'o')? Node::O:
						(c == 'X' || c == 'x')? Node::X:
						                        Node::EMPTY;
				}
			}
		}

	    OPERATION::TYPE play()
		{
			// selection
		    Node * node = this->root.best_node();

		    // lack of candidate node => simulation should stop
		    if (node == nullptr)
			    return OPERATION::STOP;

		    // expansion
		    auto next = node->expansion();
		    if (next == OPERATION::SKIP)
			    return OPERATION::SKIP;

		    // simulation
		    Node * target = node->random_child();
		    auto   winner = target->simulation();

		    // back propagation
		    target->update_status(winner);
		    
		    return OPERATION::CONTINUE;
		}
	    
	    void export_to(std::ostream &o = std::cout)
		{
			o << "digraph monte_carlo_tree_search_result {\n";
			visit(&root, o);
			o << "}";
		}

	    void visit(Node *node, std::ostream &o)
		{
			o << "    " << uintptr_t(node) << "[label = \""
			  << "Node: " << node << "\n"
			  << "Role: Player " << node->player << ", Next: " << Node::flip(node->player) << "\n"
			  << "wins/total: " << node->win << "/" << node->total << ", " << (!node->parent? 0: node->UCT()) << "\n"
			  << node->board[0][0] << node->board[0][1] << node->board[0][2] << "\n"
			  << node->board[1][0] << node->board[1][1] << node->board[1][2] << "\n"
			  << node->board[2][0] << node->board[2][1] << node->board[2][2]
			  << "\"; "
			  << "color = " << (node->player == Node::peice::O ? "blue": "red") << ";]\n";
			for (auto child: node->child)
			{
				o << "    " << uintptr_t(node)
				  << " -> " << uintptr_t(child) << "[color = " << (node->player == Node::peice::O ? "blue": "red") << "];\n";
				visit(child, o);
			}
		}
    };
}

#endif /* __MCTS_HPP__ */

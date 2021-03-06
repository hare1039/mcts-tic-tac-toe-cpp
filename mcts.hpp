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
	    
	    Node(peice p): player(p) {}

        ~Node()
        {
            for (auto &n : child)
                delete n;
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

    public: // methods
	    Node& set_board(std::array<std::array<peice, BOARD_SIZE>, BOARD_SIZE> const &src)
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

	    Node* best_node()
        {
	        if (child.empty())
                return this;
            return (**std::max_element(child.begin(), child.end(), [](const Node *A, const Node *B) {
                return A->UCT() < B->UCT();
		    })).best_node();
        }


	    OPERATION::TYPE expansion()
		{
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
		        static_cast<double>(win) / total + std::sqrt(2) * std::sqrt(std::log((parent)? parent->total: 0) / total);
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
	    
	    void export_to_dot(std::ostream &o = std::cout)
		{
			o << "digraph monte_carlo_tree_search_result {\n";
			visit_dot(&root, o);
			o << "}";
		}
	    void visit_dot(Node *node, std::ostream &o)
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
				visit_dot(child, o);
			}
		}

	    void export_to_html(std::ostream &o = std::cout)
		{
			o << "<html><head><title>monte_carlo_tree_search_result</title><link rel='stylesheet' href='//kendo.cdn.telerik.com/2018.1.221/styles/kendo.common.min.css'/><link rel='stylesheet' href='//kendo.cdn.telerik.com/2018.1.221/styles/kendo.default.min.css'/><link rel='stylesheet' href='//kendo.cdn.telerik.com/2018.1.221/styles/kendo.default.mobile.min.css'/><script src='//code.jquery.com/jquery-3.3.1.min.js'></script><script src='//kendo.cdn.telerik.com/2018.1.221/js/kendo.all.min.js'></script></head><body><div id='example'><div class='demo-section k-content'><ul id='treeview'><li> MCTS Tree root";
			visit_html(&root, o);
			o << "<li></ul></div><script>$(document).ready(function(){$('#treeview').kendoTreeView();});</script><style>.rootfolder { background-position:0 0;} .folder {background-position:0 -16px;}.pdf {background-position:0 -32px;}.html {background-position: 0 -48px;} .image {background-position:0 -64px;}</style></div></body></html>" << std::endl;
		}

	    void visit_html(Node *node, std::ostream &o)
		{
			o << "<ul>";
			o << "<li>" << node->board[0][0] << node->board[0][1] << node->board[0][2] << "<br/>"
			  << node->board[1][0] << node->board[1][1] << node->board[1][2] << "<br/>"
			  << node->board[2][0] << node->board[2][1] << node->board[2][2] << "</li>";

			for (auto child: node->child)
			{
				o << "<li> Player " << child->player
				  << ", UCT: " << child->UCT() << ", Win: " << (static_cast<double>(child->win) / child->total);
				visit_html(child, o);
				o << "</li>";
			}
			
//			if (node->child.empty())
//			{
//				o << node->board[0][0] << node->board[0][1] << node->board[0][2] << "<br/>"
//				  << node->board[1][0] << node->board[1][1] << node->board[1][2] << "<br/>"
//				  << node->board[2][0] << node->board[2][1] << node->board[2][2];
//			}
			o << "</ul>";
		}
    };
}

#endif /* __MCTS_HPP__ */

#include <vector>
#include <random>
#include <functional>
#include <algorithm>

#include <iostream>
#include <cmath> 

#define UNTIL(x) while(not(x))

namespace monte_carlo_tree_search
{
	class Node
	{
		enum peice{ EMPTY, O, X } board[4][4];
		peice flip(peice p)
		{
			if(p == EMPTY)
			{
				std::cout << "warning: flip EMPTY peice\n";
				return EMPTY;
			}
			return (p == O)? X: O;
		}
		/* E == empty
		  |   | 1 | 2 | 3 |
		  | 1 |   |   |   |
		  | 2 |   |   |   |
		  | 3 |   |   |   |
		 */
		std::random_device dev;
		std::mt19937 engine;
		std::uniform_int_distribution<int> dist;
		std::function<int(void)> random;
		int win = 0, lose = 0;
		
		int total() const { return win + lose; }
		double win_percentage() const { return (total() == 0)? 0: static_cast<double>(win) / total(); }
		double UCT(int total_eval) const
		{
			return win_percentage() + std::sqrt(2) * std::sqrt(std::log(total_eval) / total());
		}
		
		static
    	peice winner(peice eval_board[4][4])
		{
			if((eval_board[1][1] == eval_board[1][2] && eval_board[1][2] == eval_board[1][3]) ||
			   (eval_board[1][1] == eval_board[2][1] && eval_board[2][1] == eval_board[3][1]) )
				return eval_board[1][1];
			
			else if  ((eval_board[2][1] == eval_board[2][2] && eval_board[2][2] == eval_board[2][3]) ||
			          (eval_board[1][2] == eval_board[2][2] && eval_board[2][2] == eval_board[3][2]) ||
			          (eval_board[1][1] == eval_board[2][2] && eval_board[2][2] == eval_board[3][3]) ||
			          (eval_board[3][1] == eval_board[2][2] && eval_board[2][2] == eval_board[1][3]) )
				return eval_board[2][2];
			
			else if  ((eval_board[1][3] == eval_board[2][3] && eval_board[2][3] == eval_board[3][3]) ||
			          (eval_board[3][1] == eval_board[3][2] && eval_board[3][2] == eval_board[3][3]) )
				return eval_board[3][3];
			
			else
				return EMPTY;
		}
		
		Node * parent = nullptr;
		std::vector<Node *> child;

	public:
		Node(): engine(dev()), dist(1, 3), random(std::bind(dist, engine)){}
	    
		~Node()
		{
			for(auto &n : child)
				delete n;		
		}

		Node& set_board(peice src[4][4])
		{
			for(int i(1); i <= 3; i++)
				for(int j(1); j <= 3; j++)
					board[i][j] = src[i][j];
			return *this;
		}
		
		Node& add_child(Node * n)
		{
			child.push_back(n);
			n->parent = this;
			return *this;
		}

		Node* best_node(int on_step)
		{
			if(child.empty())
				return this;
			
			std::max_element(child.begin(), child.end(), [&on_step](const Node *A, const Node *B){
				return A->UCT(on_step) < B->UCT(on_step);   
			});
			return (*child.begin())->best_node(on_step);
		}

		
		void random_play(peice p)
		{
			std::function<std::tuple<int, int>(void)> gen_pos = [this]
			{
				int x(0), y(0), full(0);
				do
				{
					x = this->random();
					y = this->random();
				} UNTIL (this->board[x][y] == EMPTY || ++full == 10);
				return (full >= 10)? std::make_tuple(0, 0): std::make_tuple(x, y);
			};

			Node * child = new Node;
			int x, y;
			std::tie(x, y) = gen_pos();
			child->set_board(board);
			child->board[x][y] = p;
			this->add_child(child);

			peice eval_board[4][4];
			for(int i(1); i <= 3; i++)
				for(int j(1); j <= 3; j++)
					eval_board[i][j] = this->board[i][j];

			
			while(std::tie(x, y) = gen_pos(), x != 0)
			{
				p = flip(p);
				eval_board[x][y] = p;

				peice champ = winner(eval_board);
				if(champ == O)
				{
					
					break;
				}
				else if(champ == X)
				{
					break;
				}
			}
		}

	};

	struct Tree
	{
		Node root;
	};
}

#include <iostream>
#include <bitset>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cmath>

// #define NDEBUG
#include <cassert>

#define CHAOS 0
#define ORDER 1

#define CAN_PASS 0

#define board_m 4
#define board_w 4
#define board_d 5000

#define board_h board_w
#define board_s (board_w*board_h)
#define board_moves (2*board_s+1)
#define board_pass (2*board_s)

typedef std::pair<int,int> win_rate;

class board_iterator;

class board {
public: // temp
	friend board_iterator;
	int sq[board_s];
public:
	class iterator;
	board();
	board( const board& );
	inline int at( int r, int c ) const;
	bool is_ordered() const;
	bool is_disordered() const;
	bool is_full() const;
	int is_game_over() const;
	inline bool can_move( int r, int c ) const;
	board& do_move( int r, int c, bool s );
	board& undo_move( int r, int c );
	board after_move( int r, int c, bool s ) const;
	bool operator<( const board& ) const;
	bool operator==( const board& ) const;
};

class board_iterator {
	int i;
	board b;
public:
	void advance();
	const board& operator*() const { return b; }
	board_iterator operator++() { board_iterator itr = *this; advance(); return itr; }
    board_iterator operator++(int) { advance(); return *this; }
	bool operator==( const board_iterator& other ) { return i == other.i; } // hacky
	bool operator!=( const board_iterator& other ) { return not operator==( other ); }
	board_iterator( const board& c, bool can_pass ) { b = c; i = -1-can_pass; advance(); }
	board_iterator() { i = -3; }
};

struct next_moves {
	bool cp;
	const board* data;
	board_iterator begin() const { return board_iterator( *data, cp ); }
	board_iterator end() const { return board_iterator(); }
	next_moves( const board& b, bool can_pass = false ) { data = &b; cp = can_pass; }
};

template<uint64_t n> struct exp3 { static const uint64_t result = 3*exp3<n-1>::result; };
template<> struct exp3<0> { static const uint64_t result = 1; };
struct all_boards {
	class iterator {
		uint64_t i;
	public:
		board operator*() const {
			board b;
			uint64_t j = i, k = 0;
			const int v[3] = { 0, 1, -1 };
			while( j ) {
				b.sq[k++] = v[j % 3];
				j /= 3;
			}
			return b;
		}
		void advance() {
			++i;
			if( i >= exp3<board_s>::result )
				i = -1;
		}
		iterator operator++() { iterator itr = *this; advance(); return itr; }
		iterator operator++(int) { advance(); return *this; }
		bool operator==( iterator other ) { return i == other.i; }
		bool operator!=( iterator other ) { return i != other.i; }
		iterator( uint64_t start ) { i = start; }
	};
	iterator begin() const { return iterator(0); }
	iterator end() const { return iterator(-1); }
};

class monte_carlo_tree_search {
public:
	struct node {
		win_rate rate;
		node* children[board_moves];
	public:
		node*& get_child( int r, int c, bool symbol );
		node* get_unexplored_child( board&, bool player );
		template<double (*score_function)( win_rate, win_rate )>
		node* get_best_explored_child( board&, bool player );
		void print( board b, int use_cap, int depth );
		node();
		~node();
	};
	typedef std::vector<node*> history;
private:
	node* root;
public:
	history select( board& b, bool turn ) const;
	void back_propagate( const history& h, bool turn, bool winner );
	bool play_out( board b, bool turn ) const;
	bool simulate( board b, bool turn, int dives = 1000 );
	void print( board b, int use_cap = 5 );

	void clear();
	monte_carlo_tree_search();
	~monte_carlo_tree_search();
};

inline int board::at( int r, int c ) const {
	return sq[ board_w*r+c ];
}

bool board::is_ordered() const {
	const int offset_w = board_w - board_m; 
	// horizontals
	for( int r = 0; r < board_h; ++r ) {
		for( int c = 0; c <= offset_w; ++c ) {
			if( at( r, c ) != 0 ) {
				bool succes = true;
				for( int i = 1; i < board_m; ++i ) {
					if( at( r, c ) != at( r, c+i ) ) {
						succes = false;
						break;
					}
				}
				if( succes )
					return true;
			}
		}
	}
	// verticals
	const int offset_h = board_h - board_m; 
	for( int c = 0; c < board_w; ++c ) {
		for( int r = 0; r <= offset_h; ++r ) {
			if( at( r, c ) != 0 ) {
				bool succes = true;
				for( int i = 1; i < board_m; ++i ) {
					if( at( r, c ) != at( r+i, c ) ) {
						succes = false;
						break;
					}
				}
				if( succes )
					return true;
			}
		}
	}
	// diagonals
	for( int r = 0; r <= offset_h; ++r ) {
		for( int c = 0; c <= offset_w; ++c ) {
			if( at( r, c ) != 0 ) {
				bool succes = true;
				for( int i = 1; i < board_m; ++i ) {
					if( at( r, c ) != at( r+i, c+i ) ) {
						succes = false;
						break;
					}
				}
				if( succes )
					return true;
			}
			if( at( board_h-1-r, board_w-1-c ) != 0 ) {
				bool succes = true;
				for( int i = 1; i < board_m; ++i ) {
					if( at( board_h-1-r, board_w-1-c ) != at( board_h-1-r-i, board_w-1-c-i ) ) {
						succes = false;
						break;
					}
				}
				if( succes )
					return true;
			}
		}
	}
	return false;
}

bool board::is_disordered() const {
	for( int s = -1; s < 2; s += 2 ) {
		board c = *this;
		for( int i = 0; i < board_s; ++i ) 
			if( c.sq[i] == 0 )
				c.sq[i] = s;
		if( c.is_ordered() )
			return false;
	}
	return true;
}

bool board::is_full() const {
	for( int i = 0; i < board_s; ++i )
		if( !sq[i] )
			return false;
	return true;
}

int board::is_game_over() const {
	if( is_ordered() )
		return 1;
	if( is_disordered() )
		return -1;
	return 0;
}

inline bool board::can_move( int r, int c ) const {
	return not at( r, c );
}

board& board::do_move( int r, int c, bool s ) {
	assert( can_move( r, c ) );
	sq[ board_w*r+c ] = 1-2*(!!s);
	return *this;
}

board& board::undo_move( int r, int c ) {
	assert( not can_move( r, c ) );
	sq[ board_w*r+c ] = 0;
	return *this;
}

board board::after_move( int r, int c, bool s ) const {
	board b = *this;
	return b.do_move( r, c, s );
}

board::board() {
	for( int i = 0; i < board_s; ++i )
		sq[i] = 0;
}

board::board( const board& other ) {
	for( int i = 0; i < board_s; ++i )
		sq[i] = other.sq[i];
}

void board_iterator::advance() {
	if( i == -3 )
		return;
	if( i == -2 ) {
		i++;
		return;
	}
	if( i >= 0 )
		b.sq[i>>1] = 0;
	while( ++i < 2*board_s ) {
		if( b.sq[i>>1] == 0 ) {
			b.sq[i>>1] = 1-2*( i & 1 );
			return;
		}
	}
	i = -3;
}

bool board::operator<( const board& other ) const {
	for( int i = 0; i < board_s; ++i ) {
		if( sq[i] < other.sq[i] )
			return true;
		if( sq[i] > other.sq[i] )
			return false;
	}
	return false;
}

bool board::operator==( const board& other ) const {
	for( int i = 0; i < board_s; ++i )
		if( sq[i] != other.sq[i] )
			return false;
	return true;
}

std::ostream& operator<<( std::ostream& os, board b ) {
	for( int r = 0; r < board_h; ++r ) {
		for( int c = 0; c < board_w; ++c ) {
			switch( b.at(r,c) ) {
				case 0:
					os << ".";
					break;
				case 1:
					os << "O";
					break;
				case -1: 
					os << "X";
					break;
				default:
					os << "?";
			}
		}
		os << "\n";
	}
	return os;
}

template<board (*do_move)( board, bool )>
bool play_game( board b, bool turn, bool print = false ) {
	int result;
	while( not ( result = b.is_game_over() ) ) {
		b = do_move( b, turn );
		if( print )
			std::cout << b;
		turn = not turn;
	}
	return result == ORDER;
}

board random_move( board b, bool turn ) {
	// compute modulus
	int move_count = 0;
	for( int r = 0; r < board_h; ++r )
		for( int c = 0; c < board_w; ++c )
			if( b.can_move( r, c ) )
				move_count += 2;
	if( turn == CHAOS and CAN_PASS )
		move_count++;
	assert( move_count > 0 );
	// do move
	int sample = rand() % move_count;
	int s = sample & 1;
	int j = sample >> 1;
	for( int r = 0; r < board_h; ++r )
		for( int c = 0; c < board_w; ++c )
			if( b.can_move( r, c ) )
				if( j-- == 0 )
					return b.do_move( r, c, s );
	// pass
	assert( turn == CHAOS and CAN_PASS );
	return b;
}

constexpr double confidence_score_function( win_rate child, win_rate parent ) {
	return double( child.first - child.second ) / double( child.first ) + sqrt( 2.0 * log( double( parent.first ) ) / double( child.first ) );
}

constexpr double best_score_function( win_rate child, win_rate parent ) {
	return double( child.first - child.second ) / double( child.first );
}

monte_carlo_tree_search::node::node() {
	rate.first = rate.second = 0;
	for( int i = 0; i < board_moves; ++i )
		children[i] = nullptr;
}

monte_carlo_tree_search::node::~node() {
	for( int i = 0; i < board_moves; ++i )
		if( children[i] )
			delete children[i];
}

monte_carlo_tree_search::node*& monte_carlo_tree_search::node::get_child( int r, int c, bool symbol ) {
	return children[symbol*board_s+r*board_w+c];
}

monte_carlo_tree_search::node* monte_carlo_tree_search::node::get_unexplored_child( board& b, bool turn ) {
	int movec = 0;
	if( CAN_PASS and turn == CHAOS and children[board_pass] == nullptr )
		movec++;
	for( int r = 0; r < board_h; ++r )
		for( int c = 0; c < board_w; ++c )
			if( b.can_move( r, c ) )
				for( int s = 0; s < 2; ++s )
					if( get_child( r, c, s ) == nullptr )
						movec++;
	if( movec == 0 )
		return nullptr;
	int choice = rand() % movec;
	for( int r = 0; r < board_h; ++r )
		for( int c = 0; c < board_w; ++c )
			if( b.can_move( r, c ) )
				for( int s = 0; s < 2; ++s )
					if( get_child( r, c, s ) == nullptr and (choice--) == 0 ) {
						b.do_move( r, c, s );
						return get_child( r, c, s ) = new node;
					}
	assert( choice == 0 and children[board_pass] == nullptr );
	return children[board_pass] = new node;
}

template<double (*score_function)( win_rate, win_rate )>
monte_carlo_tree_search::node* monte_carlo_tree_search::node::get_best_explored_child( board& b, bool turn ) {
	double bscore = -1.0;
	int br, bc, bs;
	
	for( int r = 0; r < board_h; ++r ) {
		for( int c = 0; c < board_w; ++c ) {
			if( b.can_move( r, c ) ) {
				for( int s = 0; s < 2; ++s ) {
					if( get_child( r, c, s ) == nullptr )
						return nullptr;
					double score = score_function( get_child( r, c, s )->rate, rate );
					if( score > bscore ) {
						bscore = score;
						br = r;
						bc = c;
						bs = s;
					}
				}
			}
		}
	}
	if( CAN_PASS and turn == CHAOS ) {
		if( children[board_pass] == nullptr )
			return nullptr;
		if( score_function( children[board_pass]->rate, rate ) > bscore )
			return children[board_pass];
	}		
	assert( bscore > -0.5 );

	b.do_move( br, bc, bs );
	return get_child( br, bc, bs );
}

monte_carlo_tree_search::history monte_carlo_tree_search::select( board& b, bool turn ) const {
	history h = { root };
	node* current = root;
	while( current != nullptr and not b.is_game_over() ) {
		current = current->get_best_explored_child<confidence_score_function>( b, turn );
		turn = !turn;
		h.push_back( current );
	}
	return h;
}

void monte_carlo_tree_search::back_propagate( const history& h, bool turn, bool winner ) {
	for( node* n : h ) {
		n->rate.first += 1;
		n->rate.second += ( turn == winner );
		turn = !turn;
	}
}

bool monte_carlo_tree_search::play_out( board b, bool turn ) const {
	return play_game<random_move>( b, turn );
}

bool monte_carlo_tree_search::simulate( board b, bool turn, int dives ) {
	int result;
	while( not ( result = b.is_game_over() ) ) {
		for( int i = 0; i < dives; ++i ) {
			board c = b;
			history h = select( c, turn );
			bool winner = c.is_ordered();

			if( h.back() == nullptr ) { // there are unexplored children
				bool cturn = ( turn + h.size() ) % 2;
				h.back() = h.at( h.size()-2 )->get_unexplored_child( c, cturn ); 
				winner = play_out( c, !cturn );
			}
			
			back_propagate( h, turn, winner );
		}
		// print( b );

		if( false ) { // printing
			for( node* n : root->children ) {
				std::cout << n;
				if( n )
					std::cout << " " << n->rate.second << ":" << n->rate.first;
				std::cout << "\n";
			}
		}
		assert( root->get_best_explored_child<best_score_function>( b, turn ) != nullptr );
		turn = !turn;

		if( false ) { // printing
			std::cout << (( turn == ORDER ) ? "\033[32m" : "\033[31m" ) << b << "\033[0m" << root->rate.second << ":" << root->rate.first << "\n---------" << std::endl;
		}
		clear();
	}
	return result == ORDER;
}

void monte_carlo_tree_search::print( board b, int use_cap ) {
	if( root ) {
		std::cout << root->rate.second << ":" << root->rate.first << std::endl;
		root->print( b, use_cap, 1 );
	}
}

void monte_carlo_tree_search::node::print( board b, int use_cap, int depth ) {
	bool a = false;
	for( int r = 0; r < board_h; ++r ) {
		for( int c = 0; c < board_w; ++c ) {
			if( b.can_move( r, c ) ) {
				for( int s = 0; s < 2; ++s ) {
					node* n = get_child( r, c, s );
					if( n == nullptr )
						continue;
					if( n->rate.first >= use_cap )
						continue;
					if( not a ) {
						a = true;
						for( int i = 0; i < depth; ++i )
							std::cout << " ";
					}
					std::cout << n->rate.second << ":" << n->rate.first << " ";
				}
			}
		}
	}
	if( a )
		std::cout << "\n";
	for( int r = 0; r < board_h; ++r ) {
		for( int c = 0; c < board_w; ++c ) {
			if( b.can_move( r, c ) ) {
				for( int s = 0; s < 2; ++s ) {
					node* n = get_child( r, c, s );
					if( n == nullptr )
						continue;
					if( n->rate.first < use_cap )
						continue;
					b.do_move( r, c, s );
					if( depth == 1 )
						std::cout << b;
					for( int i = 0; i < depth; ++i )
						std::cout << " ";
					std::cout << n->rate.second << ":" << n->rate.first << " (" << ( double( n->rate.second ) / double( n->rate.first ) ) << ")\n";
					n->print( b, use_cap, depth+1 );
					b.undo_move( r, c );
				}
			}
		}
	}
}

void monte_carlo_tree_search::clear() {
	if( root )
		delete root;
	root = new node();
}

monte_carlo_tree_search::monte_carlo_tree_search() {
	root = new node();
}

monte_carlo_tree_search::~monte_carlo_tree_search() {
	if( root )
		delete root;
}

int main() {
	int rc = 0;
	for( int i = 0; i < 100; ++i ) {
		std::cout << "Game " << i << std::endl;
		monte_carlo_tree_search tree;
		rc += tree.simulate( board(), ORDER, board_d );
		std::cout << rc << "/" << (i+1) << std::endl;
	}
}
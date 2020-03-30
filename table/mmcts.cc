#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#define NDEBUG
#include <cassert>

#define NOPLAYER -1
#define CHAOS 0
#define ORDER 1

// The following values should be defined via compiler flags
//#define CAN_PASS 1
//#define PASS_PLAYER CHAOS
//#define board_m 5
//#define board_w 6
//#define board_d 5000

#define board_h board_w
#define board_s (board_w*board_h)
#define board_moves (2*board_s+1)
#define board_pass (2*board_s)

typedef std::pair<int,int> win_rate;

class board {
	int sq[board_s];
public:
	board();
	board( const board& );
	inline int at( int r, int c ) const;
	bool is_ordered() const;
	bool is_disordered() const;
	int game_over_state() const;
	inline bool can_move( int r, int c ) const;
	board& do_move( int r, int c, bool s );
	bool operator==( const board& ) const;
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
		node& operator=( const node& ); // here to satisfy the g++ warnings
		node( const node& ); // here to satisfy the g++ warnings
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
	bool simulate( board b, bool turn, int dives, bool print = false );
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
			if( at( r, board_w-1-c ) != 0 ) {
				bool succes = true;
				for( int i = 1; i < board_m; ++i ) {
					if( at( r, board_w-1-c ) != at( r+i, board_w-1-c-i ) ) {
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
	for( int s = 0; s < 2; ++s ) {
		board c = *this;
		for( int i = 0; i < board_s; ++i ) 
			if( c.sq[i] == 0 )
				c.sq[i] = s+1;
		if( c.is_ordered() )
			return false;
	}
	return true;
}

int board::game_over_state() const {
	if( is_ordered() )
		return ORDER;
	if( is_disordered() )
		return CHAOS;
	return -1;
}

inline bool board::can_move( int r, int c ) const {
	return at( r, c ) == 0;
}

board& board::do_move( int r, int c, bool s ) {
	assert( can_move( r, c ) );
	sq[ board_w*r+c ] = 1+(!!s);
	return *this;
}

board::board() {
	for( int i = 0; i < board_s; ++i )
		sq[i] = 0;
}

board::board( const board& other ) {
	for( int i = 0; i < board_s; ++i )
		sq[i] = other.sq[i];
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
			switch( b.at( r, c ) ) {
				case 0:
					os << ".";
					break;
				case 1:
					os << "O";
					break;
				case 2: 
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
	while( ( result = b.game_over_state() ) == NOPLAYER ) {
		b = do_move( b, turn );
		if( print )
			std::cout << b;
		turn = not turn;
	}
	return result;
}

board random_move( board b, bool turn ) {
	// compute modulus
	int move_count = 0;
	for( int r = 0; r < board_h; ++r )
		for( int c = 0; c < board_w; ++c )
			if( b.can_move( r, c ) )
				move_count += 2;
	if( turn == PASS_PLAYER and CAN_PASS )
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
	assert( turn == PASS_PLAYER and CAN_PASS );
	return b;
}

constexpr double confidence_score_function( win_rate child, win_rate parent ) {
	return double( child.first - child.second ) / double( child.first ) + sqrt( 2.0 * log( double( parent.first ) ) / double( child.first ) );
}

constexpr double best_score_function( win_rate child, win_rate parent ) {
	return double( child.first - child.second ) / double( child.first );
}

monte_carlo_tree_search::node& monte_carlo_tree_search::node::operator=( const node& other ) {
	// If you ever call this function you have a problem
	assert( false );
	rate = other.rate;
	for( int i = 0; i < board_moves; ++i )
		children[i] = other.children[i];
	return *this;
}

monte_carlo_tree_search::node::node( const node& other ) {
	operator=( other );
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
	if( CAN_PASS and turn == PASS_PLAYER and children[board_pass] == nullptr )
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
					if( ( get_child( r, c, s ) == nullptr ) and ( (choice--) == 0 ) ) {
						b.do_move( r, c, s );
						return get_child( r, c, s ) = new node;
					}
	assert( choice == 0 and children[board_pass] == nullptr );
	return children[board_pass] = new node;
}

template<double (*score_function)( win_rate, win_rate )>
monte_carlo_tree_search::node* monte_carlo_tree_search::node::get_best_explored_child( board& b, bool turn ) {
	double bscore = -1.0;
	int br = -1, bc = -1, bs = -1;
	
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
	if( CAN_PASS and turn == PASS_PLAYER ) {
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
	while( ( current != nullptr ) and ( b.game_over_state() == NOPLAYER ) ) {
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

bool monte_carlo_tree_search::simulate( board b, bool turn, int dives, bool print ) {
	int result;
	while( ( result = b.game_over_state() ) == NOPLAYER ) {
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
		node* choice = root->get_best_explored_child<best_score_function>( b, turn );
		turn = !turn;
		assert( choice != nullptr );

		if( print ) {
			std::cout << ( turn ? "\033[32m" : "\033[31m" ) << b << "\033[0m" << choice->rate.second << ":" << choice->rate.first << "\n---------" << std::endl;
		}

		clear();
	}
	return result;
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

int main( int argc, char* argv[] ) {
	if( argc <= 1 ) {
		std::cout << "Error!" << std::endl;
		return 1;
	}
	srand(uint(atoi(argv[1])));
	monte_carlo_tree_search tree;
	std::cout << int( tree.simulate( board(), PASS_PLAYER, board_d, false ) ) << " " << argv[1] << std::endl;
	return 0;
}
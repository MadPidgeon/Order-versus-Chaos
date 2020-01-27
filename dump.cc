#include <iostream>
#include <bitset>
#include <fstream>

#define popcount __builtin_popcount

#define NDEBUG
#include <cassert>

#define CHAOS 0
#define ORDER 1

#define CAN_PASS 1

typedef uint32_t board;

constexpr int32_t _3pow16 = 43046721;
constexpr int32_t total_bytes = (_3pow16+7)/8;
constexpr int32_t win_line_count = 10;

constexpr board win_line[win_line_count] = {
	0b1000100010001000,
	0b0100010001000100,
	0b0010001000100010,
	0b0001000100010001,
	0b1111000000000000,
	0b0000111100000000,
	0b0000000011110000,
	0b0000000000001111,
	0b1000010000100001,
	0b0001001001001000
};

constexpr bool is_ordered( board b ) {
	bool v = false;
	for( int c = 0; c < 2; ++c ) {
		#pragma unroll
		for( int i = 0; i < win_line_count; ++i )
			v |= ( ( b & win_line[i] ) == win_line[i] );
		if( v )
			return true;
		b >>= 16;
	}
	return false;
}

constexpr bool is_full( board b ) {
	return popcount( b ) == 16;
}

constexpr bool is_sane( board b ) {
	return ( b & ( b >> 16 ) ) == 0;
}

constexpr board index_to_board( int32_t index ) {
	board r = 0;
	int digit = 0;
	for( int i = 0; i < 16; ++i ) {
		digit = index % 3;
		index /= 3;
		r |= ( digit & 1 ) << i;
		r |= ( digit >> 1 ) << (i+16);
	}
	return r;
}

constexpr int32_t board_to_index( board b ) {
	int32_t index = 0;
	int digit = 0;
	for( int i = 15; i >= 0; --i ) {
		digit = ( (b>>i) & 1 ) + 2*( (b>>(i+16)) & 1 );
		index = 3*index+digit;
	}
	return index;
}

constexpr int32_t move_on_index( int32_t index, int i, bool s ) {
	int digit = (!!s)+1;
	for( int j = 0; j < i; ++j )
		digit *= 3;
	return index + digit;
}

constexpr bool can_move_on_board( board b, int i ) {
	return ( b & ( 0x10001 << i ) ) == 0;
}

constexpr board move_on_board( board b, int i, bool s ) {
	assert( can_move_on_board( b, i ) );
	return b | ( 1 << ( i+16*s ) );
}

bool conversion_correct() {
	for( int32_t val = _3pow16-1; val >= 0; --val ) {
		if( board_to_index(index_to_board(val)) != val ) {
			std::cout << val << std::endl;
			return 0;
		}
	}
	return 1;
}

std::bitset<_3pow16> memo[2];

bool fill_memo( int32_t index, board b, bool p ) {
	assert( is_sane(b) );
	// check order
	if( is_ordered( b ) )
		return memo[p][index] = ORDER;
	// check chaos
	if( is_full( b ) )
		return memo[p][index] = CHAOS;
	// pass
	if( CAN_PASS and p == CHAOS and memo[ORDER][index] == CHAOS )
		return memo[p][index] = CHAOS;
	// play
	for( int i = 0; i < 16; ++i )
		if( can_move_on_board( b, i ) )
			for( int j = 0; j < 2; ++j )
				if( memo[!p][move_on_index(index,i,j)] == p )
					return memo[p][index] = p;
	return memo[p][index] = !p;
}

void fill_all_memo() {
	for( int32_t index = _3pow16-1; index >= 0; --index ) {
		board b = index_to_board( index );
		fill_memo( index, b, ORDER ); // should be done first since chaos can pass
		fill_memo( index, b, CHAOS );
	}
}

void print_board( board b ) {
	for( int i = 0; i < 16; ++i ) {
		if( b & (1<<i) )
			std::cout << "O";
		else if( b & (1<<(i+16)) )
			std::cout << "X";
		else
			std::cout << ".";
		if( ( i & 3 ) == 3 )
			std::cout << std::endl;
	}
}


int main() {
	assert( conversion_correct() );
	std::cout << "Computing..." << std::endl;
	fill_all_memo();
	std::cout << "Writing..." << std::endl;
	std::ofstream p1("order.txt");
	p1 << memo[ORDER];
	std::ofstream p2("chaos.txt");
	p2 << memo[CHAOS];
}


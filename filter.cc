#include <fstream>
#include <iostream>
#include <bitset>
#include <vector>
#include <map>

#define popcount __builtin_popcount

#define NDEBUG
#include <cassert>

using namespace std;

typedef uint32_t board;

constexpr int32_t _3pow16 = 43046721;
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

bitset<_3pow16> order_win;

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

constexpr bool is_done( board b ) {
	for( int i = 0; i < win_line_count; ++i ) {
		if( ( b & win_line[i] ) and ( ( b >> 16 ) & win_line[i] ) )
			continue;
		return false;
	}
	return true;
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

constexpr board toggle( board b ) {
	return ( b << 16 ) | ( b >> 16 );
}

constexpr board mirror( board b ) {
	board s = b & 0b10001000100010001000100010001000;
	board t = b & 0b01000100010001000100010001000100;
	board u = b & 0b00100010001000100010001000100010;
	board v = b & 0b00010001000100010001000100010001;
	return (s >> 3) | (t >> 1) | (u << 1) | (v << 3);
}

constexpr board rotate( board b ) {
	return 
	 (( b & 0b10000000000000001000000000000000 ) >> 3  )
	|(( b & 0b01000000000000000100000000000000 ) >> 6  )
	|(( b & 0b00100000000000000010000000000000 ) >> 9  )
	|(( b & 0b00010000000000000001000000000000 ) >> 12 )
	|(( b & 0b00001000000000000000100000000000 ) << 2  )
	|(( b & 0b00000100000000000000010000000000 ) >> 1  )
	|(( b & 0b00000010000000000000001000000000 ) >> 4  )
	|(( b & 0b00000001000000000000000100000000 ) >> 7  )
	|(( b & 0b00000000100000000000000010000000 ) << 7  )
	|(( b & 0b00000000010000000000000001000000 ) << 4  )
	|(( b & 0b00000000001000000000000000100000 ) << 1  )
	|(( b & 0b00000000000100000000000000010000 ) >> 2  )
	|(( b & 0b00000000000010000000000000001000 ) << 12 )
	|(( b & 0b00000000000001000000000000000100 ) << 9  )
	|(( b & 0b00000000000000100000000000000010 ) << 6  )
	|(( b & 0b00000000000000010000000000000001 ) << 3  );
}

constexpr board invert( board b ) {
	return 
	 (( b & 0b10100000101000001010000010100000 ) >> 5  )
	|(( b & 0b01010000010100000101000001010000 ) >> 3  )
	|(( b & 0b00001010000010100000101000001010 ) << 3  )
	|(( b & 0b00000101000001010000010100000101 ) << 5  );
}

constexpr board canonical( board b ) {
	board c = b;
	for( int s = 0; s < 2; ++s ) {
		for( int t = 0; t < 2; ++t ) {
			for( int u = 0; u < 2; ++ u ) {
				for( int v = 0; v < 4; ++v ) {
					if( b < c )
						c = b;
					b = rotate( b );
				}
				b = toggle( b );
			}
			b = invert( b );
		}
		b = mirror( b );
	}
	return c;
}

bitset<_3pow16> order_data;

template<board (*T)(board)>
bool test_operation() {
	for( int32_t i = 0; i < _3pow16; ++i ) {
		board b = index_to_board( i );
		board c = T( b );
		int32_t j = board_to_index( c );
		if( order_data.test( i ) != order_data.test( j ) )
			return false;
	}
	return true;
}

bool test_operations() {
	std::cout << "Testing symmetries..." << std::endl;
	return test_operation<toggle>() && test_operation<mirror>() && test_operation<rotate>() && test_operation<invert>();
}

int main() {
	cout << "Reading win data..." << endl << boolalpha;
	ifstream order_data("order.txt");
	order_data >> order_win;
	order_data.close();

	assert( test_operations() );
	
	cout << "Writing JSON..." << endl;
	ofstream json_data("order.json");
	json_data << "{ \"data\" : [\n";
	bool first = true;
	for( int32_t i = 0; i < _3pow16; ++i ) {
		board b = index_to_board( i );
		if( is_ordered( b ) )
			continue;
		if( is_done( b ) )
			continue;
		if( b != canonical( b ) )
			continue;
		board c = b;

		if( first )
			first = false;
		else
			json_data << ",\n";

		json_data << "[";
		for( int i = 0; i < 16; ++i ) {
			if( i != 0 )
				json_data << ",";
			if( c & 0b0000000000000001 )
				json_data << "0";
			else if( c & 0b10000000000000000 )
				json_data << "2";
			else
				json_data << "1";
			c >>= 1;
		}
		json_data << "]";
	}
	json_data << "\n], \"target\" : [\n";

	first = true;
	for( int32_t i = 0; i < _3pow16; ++i ) {
		board b = index_to_board( i );
		if( is_ordered( b ) )
			continue;
		if( is_done( b ) )
			continue;
		if( b != canonical( b ) )
			continue;

		if( first )
			first = false;
		else
			json_data << ",\n";

		json_data << ( -1+2*order_win.test(i) );
	}
	json_data << "\n]}\n";
}
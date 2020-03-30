#include <iostream>
#include <vector>
#include <utility>

const int dx[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
const int dy[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };

typedef std::pair<int,int> ii;
typedef std::vector<int> vi;

int adj[8][8][8];
ii edges[64*4];

int main() {
	int nedg = 0;
	for( int d = 0; d < 4; ++d ) {
		for( int x = 0; x < 8; ++x ) {
			for( int y = 0; y < 8; ++y ) {
				int nx = (x + dx[d] + 8) % 8;
				int ny = (y + dy[d] + 8) % 8;
				adj[ x][ y][  d] = nedg;
				adj[nx][ny][4+d] = nedg;
				edges[nedg++] = ii( 8*x + y, 8*nx + ny );
			}
		}
	}

	std::cout << "p cnf " << ( 64*4 ) << " " << ( 8*4 + 8*8*8*7/2 ) << std::endl;

	for( int d = 0; d < 4; ++d ) {
		for( int x = 0; x < 8; ++x ) {
			int sx = x, sy = 0;
			if( d == 3 ) 
				std::swap(sx, sy);

			vi constr;
			for( int i = 0; i < 8; ++i ) {
				int vx = (sx + i*dx[d] + 8) % 8;
				int vy = (sy + i*dy[d] + 8) % 8;
				constr.push_back( adj[vx][vy][d] );
			}

			for( int x : constr ) 
				std::cout << (x+1) << " ";
			std::cout << " 0\n";
		}
	}

	for( int x = 0; x < 8; ++x )
		for( int y = 0; y < 8; ++y )
			for( int d = 0; d < 8; ++d )
				for( int e = 0; e < d; ++e )
					std::cout << "-" << ( adj[x][y][d] + 1 ) << " -" << ( adj[x][y][e] + 1 ) << " 0\n";
	
	return 0;
}

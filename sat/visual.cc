#include <iostream>
#include <string>

const int dx[4] = { -1, 0, 1, 1 };
const int dy[4] = { 1, 1, 1, 0 };
const std::string arrow[] = { "↗", "→", "↘", "↓" , "↙", "←", "↖", "↑" };

int main() {
	int G[8][8];
	std::string S;
	while( std::cin >> S ) {
		if( S[0] == 's' or S[0] == 'S' or S[0] == 'v' or S[0] == '0' )
			continue;

		int num = stoi( S );
		if( num > 0 ) {
			num--;
			int d = num/64;
			int x = (num%64) / 8;
			int y = (num%64) % 8;

			int nx = (x + dx[d] + 8) % 8;
			int ny = (y + dy[d] + 8) % 8;

			G[x][y]   = d;
			G[nx][ny] = d+4;   
		}
	}

	for( int i = 0; i < 8; ++i ) {
		for( int j = 0; j < 8; ++j )
			std::cout << arrow[G[i][j]] << " ";
		std::cout << std::endl;
	}

	return 0;
}

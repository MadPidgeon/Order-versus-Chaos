from math import *
import random
import copy


class OvcState:

    def __init__(self, x, y):
        self.player_just_moved = 2
        self.width = x
        self.length = y
        self.bord = [['   ' for x in range(self.width)] for y in range(self.width)]

    def clone(self):
        st = OvcState(self.width, self.length)
        st.player_just_moved = self.player_just_moved
        for i in range(st.width):
            for j in range(st.length):
                st.bord[i][j] = self.bord[i][j]
        return st

    def make_empty(self):
        for i in range(self.width):
            for j in range(self.length):
                self.bord[i][j] = '   '

    def is_full(self):
        for i in range(self.width):
            for j in range(self.length):
                if self.bord[i][j] == '   ':
                    return False
        return True

    @property
    def in_order(self):

        to_win = 6      # Adjust to the row length to win

        def horizontal(x, y, length):
            # Starts at a certain height, but x-index 0. If one field is not the same as the one before
            # or empty, it is not in order.
            if length == to_win:
                return True
            if self.bord[x + 1][y] == self.bord[x][y] and self.bord[x + 1][y] != '   ':
                return horizontal(x + 1, y, length + 1)
            return False
            # except(IndexError):
            #     return False

        def vertical(x, y, length):
            if length == to_win:
                return True
            if self.bord[x][y + 1] == self.bord[x][y] and self.bord[x][y + 1] != '   ':
                return vertical(x, y + 1, length + 1)
            return False

        def diagonal_down(x, y, length):
            if length == to_win:
                return True
            if self.bord[x + 1][y + 1] == self.bord[x][y] and self.bord[x + 1][y + 1] != '   ':
                return diagonal_down(x + 1, y + 1, length + 1)
            return False

        def diagonal_up(x, y, length):
            if length == to_win:
                return True
            if self.bord[x + 1][y - 1] == self.bord[x][y] and self.bord[x + 1][y - 1] != '   ':
                return diagonal_up(x + 1, y - 1, length + 1)
            return False

        # Checks if in order horizontally, vertically and diagonally. Because we have to make a row of 4, we
        # start at 0 and it must reach the other end of the board

        for i in range(self.length):
            # Checks every height
            for j in range(self.width + 1 - to_win):
                if horizontal(j, i, 1):
                    return True

        for i in range(self.width):
            # Checks every possible vertical line
            for j in range(self.length + 1 - to_win):
                if vertical(i, j, 1):
                    return True

        for i in range(self.width + 1 - to_win):
            for j in range(self.length + 1 - to_win):
                if diagonal_down(i, j, 1):
                    return True

        for i in range(self.width + 1 - to_win):
            for j in range(self.length + 1 - to_win):
                if diagonal_up(i, self.length - (1 + j), 1):
                    # From left top to right bottom
                    return True

        return False

    def get_moves(self):
        # Returns a list of moves you can make
        possible_moves = []
        for i in range(self.width):
            for j in range(self.length):
                for symbol in ['X', 'O']:
                    if self.bord[i][j] == '   ':
                        possible_moves.append((i, j, symbol))
        return possible_moves

    def do_move(self, i, j, symbol):
        # Places a symbol at a coordinate
        self.bord[i][j] = ' ' + symbol + ' '
        self.player_just_moved = 3 - self.player_just_moved

    def get_result(self, player):
        # Updates the states for the player that made that state
        if player == 1:
            if self.in_order:
                return 1.0
            else:
                return 0.0
        else:
            if self.in_order:
                return 0.0
            else:
                return 1.0

    def print_bord(self):   # Adjust to board size
        for i in range(self.length):
            print('---------------------------------')
            print('|' + self.bord[0][i] + '|' + self.bord[1][i] + '|' + self.bord[2][i] + '|' + self.bord[3][i] + '|' +
                  self.bord[4][i] + '|' + self.bord[5][i] + '|' + self.bord[6][i] + '|' + self.bord[7][i] + '|')
        print('---------------------------------')


class Node:

    def __init__(self, move_x=None, move_y=None, move_symbol=None, parent=None, state=None):
        self.move = (move_x, move_y, move_symbol)  # the move that got us to this node - "None" for the root node
        self.parent_node = parent  # "None" for the root node
        self.child_nodes = []
        self.wins = 0
        self.visits = 0
        self.untried_moves = state.get_moves()  # future child nodes
        self.player_just_moved = state.player_just_moved  # the only part of the state that the Node needs later

    def uct_select_child(self):
        """ Use the UCB1 formula to select a child node. Often a constant UCTK is applied so we have
            lambda c: c.wins/c.visits + UCTK * sqrt(2*log(self.visits)/c.visits to vary the amount of
            exploration versus exploitation.
        """
        s = sorted(self.child_nodes, key=lambda c: c.wins / c.visits + sqrt(2 * log(self.visits) / c.visits))[-1]
        return s

    def add_child(self, m, s):
        """ Remove m from untriedMoves and add a new child node for this move.
            Return the added child node
        """
        n = Node(move_x=m[0], move_y=m[1], move_symbol=m[2], parent=self, state=s)
        self.untried_moves.remove(m)
        self.child_nodes.append(n)
        return n

    def update(self, result):
        """ Update this node - one additional visit and result additional wins. result must be from the viewpoint of playerJustmoved.
        """
        self.visits += 1
        self.wins += result


def uct(rootstate, itermax, verbose=False):
    """ Conduct a UCT search for itermax iterations starting from rootstate.
            Return the best move from the rootstate.
            Assumes 2 alternating players (player 1 starts), with game results in the range [0.0, 1.0]."""

    root_node = Node(state=rootstate)

    for i in range(itermax):
        node = root_node
        state = rootstate.clone()

        # Select
        while node.untried_moves == [] and node.child_nodes != []:  # node is fully expanded and non-terminal
            node = node.uct_select_child()
            state.do_move(node.move[0], node.move[1], node.move[2])

        # Expand
        if node.untried_moves != []:  # if we can expand (i.e. state/node is non-terminal)
            m = random.choice(node.untried_moves)
            state.do_move(m[0], m[1], m[2])
            node = node.add_child(m, state)  # add child and descend tree

        # Rollout - this can often be made orders of magnitude quicker using a state.GetRandomMove() function
        while state.get_moves() != [] and not state.in_order:  # while state is non-terminal
            m = random.choice(state.get_moves())
            state.do_move(m[0], m[1], m[2])

        # Backpropagate
        while node is not None:  # backpropagate from the expanded node and work back to the root node
            node.update(state.get_result(
                node.player_just_moved))  # state is terminal. Update node with result from POV of node.playerJustMoved
            node = node.parent_node

    # Output some information about the tree - can be omitted

    return sorted(root_node.child_nodes, key=lambda c: c.visits)[-1].move  # return the move that was most visited

def uct_play_game():
    """ Play a sample game between two UCT players where each player gets a different number
        of UCT iterations (= simulations = tree nodes).
    """
    state = OvcState(8, 8)
    # state.print_bord()

    best_moves = []
    times_won = [0, 0]
    while state.get_moves() != [] and not state.in_order:
        if state.player_just_moved == 1:
            m = uct(rootstate=state, itermax=3000, verbose=False)  # play with values for itermax and verbose = True
        else:
            m = uct(rootstate=state, itermax=3000, verbose=False)
        print(state.player_just_moved)
        print("Best Move: " + str(m) + "\n")
        state.do_move(m[0], m[1], m[2])
        # state.print_bord()
        best_moves.append(m)
    if state.get_result(state.player_just_moved) == 1.0:
        print("Player " + str(state.player_just_moved) + " wins!")
        if state.player_just_moved == 1:
            times_won[0] += 1
        else:
            times_won[1] += 1
    elif state.get_result(state.player_just_moved) == 0.0:
        print("Player " + str(3 - state.player_just_moved) + " wins!")
        if 3 - state.player_just_moved == 1:
            times_won[0] += 1
        else:
            times_won[1] += 1
    else: print("Nobody wins!")
    return best_moves, times_won


if __name__ == "__main__":
    """ Play a single game to the end using UCT for both players. 
    """
    moves = []
    order_wins = 0
    chaos_wins = 0
    for i in range(100):
        result = uct_play_game()
        moves.append(result[0])
        order_wins += result[1][0]
        chaos_wins += result[1][1]
        print('order wins: ', order_wins, 'chaos wins: ', chaos_wins)
    print(moves)
    print(order_wins, chaos_wins)



//#define DEBUG
#undef DEBUG

#ifdef DEBUG
#include <ctime>
#endif

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include <queue>
#include <unordered_set>

using namespace std;

#define SOLUTION {1,2,3,4,5,6,7,8,0}
typedef unsigned long long ull_t;

struct Move {
    int x;
    int y;
    Move(int x_, int y_): x(x_), y(y_) {}
    string GetString() {
        if (x == 0) {
            if (y == 1)
                return "D";
            return "U";
        } else if (x == 1)
            return "R";
        return "L";
    }
};

struct TilePosition {
    int position;
    TilePosition(int position_): position(position_) {}
    TilePosition(int x_, int y_, int N_): position(y_*N_+x_) {}
    pair<int, int> positionInBoard(int N) { return make_pair(position-(position/N)*N, position/N); }
    TilePosition move (Move move, int N) {
        pair<int, int> pos = positionInBoard(N);
        pos.first += move.x;
        pos.second += move.y;
        return TilePosition(pos.first + pos.second * N);
    }
};

class Solver;

class Board {
    friend Solver;
    
    int N;
    int level;
    vector<int> tiles;
    string movements;
    TilePosition zeroPosition;
    
    int CalculateManhattanDistance();
    vector<Move> GetAvailableMoves();
    shared_ptr<Board> GetNextBoardWithMove (Move move);
    
    ull_t hash();
    weak_ptr<Solver> solver;
    
public:
    
    bool IsSolvable();
    int GetHeuristic();
    bool operator==(const Board& other);
    Board(vector<int> tiles_, int N_, int level_, const weak_ptr<Solver> solver_, string movements_);
    vector<shared_ptr<Board>> GetChildren();
};

Board::Board(vector<int> tiles_, int N_, int level_, const weak_ptr<Solver> solver_, string movements_): tiles(tiles_), N(N_), level(level_), solver(solver_), zeroPosition(TilePosition((int)(find(tiles.begin(), tiles.end(), 0) - tiles.begin()))), movements(movements_) {}

bool comp (shared_ptr<Board> first, shared_ptr<Board> second) {
    return first->GetHeuristic() > second->GetHeuristic();
}

class Solver {
    friend Board;
    priority_queue<shared_ptr<Board>, vector<shared_ptr<Board>>, decltype(&comp)> open;
    unordered_set<ull_t> used;
    
    void CalculateFactorials();
    
public:
    array<ull_t, 16> factorials;
    array<ull_t, 16>& GetFactorials() const;
    
    bool operator() (Board left, Board right) {
        return left.GetHeuristic() < right.GetHeuristic();
    }
    Board getSolution();
    Solver () {
        factorials[0] = 0;
        ull_t value = 1;
        for (int i = 1; i <= 15; i++) {
            value *= i;
            factorials[i] = value;
        }
    }
    void Solve(shared_ptr<Board> board);
};

vector<shared_ptr<Board>> Board::GetChildren() {
    vector<shared_ptr<Board>> result;
    for (auto move: GetAvailableMoves()) {
        result.push_back(GetNextBoardWithMove(move));
    }
    return result;
}

ull_t Board::hash() {
    ull_t result = 0;
    long factor = N*N - 1;
    vector<int> tiles_;
    tiles_.insert(tiles_.end(), tiles.begin(), tiles.end());
    for (int elem = 0; elem < N*N; elem++) {
        result += solver.lock()->factorials[factor] * (find(tiles_.begin(), tiles_.end(), elem) - tiles_.begin());
        tiles_.erase(find(tiles_.begin(), tiles_.end(), elem));
        factor--;
    }
    return result;
}

bool Board::IsSolvable() {
    int count = 0;
    for (int i = 0; i < N*N; i++) {
        int element = tiles[i];
        int temp = 0;
        if (element <= 1) continue;
        for (int j = 0; j < i; j++) {
            if (tiles[j] != 0 && tiles[j] < tiles[i])
                temp++;
        }
        count += element - temp - 1;
    }
    return count % 2 == 0;
}
void Solver::Solve(shared_ptr<Board> board) {
    if (!board->IsSolvable()) {
        cout << -1 << endl;
        return;
    }
    open = priority_queue<shared_ptr<Board>, vector<shared_ptr<Board>>, decltype(&comp)> (&comp);
    used = unordered_set<ull_t>();
    open.push(board);
    while (true) {
        auto current = open.top();
        open.pop();
        if (current->tiles == vector<int>({1,2,3,4,5,6,7,8,0})) {
            cout << current->level << endl << current->movements << endl;
            return;
        }
        auto children = current->GetChildren();
        for (auto ancestor: children) {
            if (!(*ancestor == *current) && used.find(ancestor->hash()) == used.end()) {
                open.push(ancestor);
            }
        }
        used.insert(current->hash());
    }
    return;
}

bool Board::operator==(const Board &other) {
    return tiles == other.tiles;
}

int Board::GetHeuristic() {
    return level + CalculateManhattanDistance();
}

shared_ptr<Board> Board::GetNextBoardWithMove(Move move) {
    TilePosition resulting_zero_position = zeroPosition.move(move, N);
    vector<int> new_tiles = tiles;
    iter_swap(new_tiles.begin() + zeroPosition.position, new_tiles.begin()+resulting_zero_position.position);
    return make_shared<Board>(*new Board(new_tiles, N, level+1, solver, movements+move.GetString()));
}

vector<Move> Board::GetAvailableMoves() {
    vector<Move> result;
    TilePosition zero_pos = zeroPosition;
    int zero_x = zero_pos.positionInBoard(N).first;
    int zero_y = zero_pos.positionInBoard(N).second;
    if (zero_x < N - 1) {
        result.push_back(Move(1, 0));
    }
    if (zero_y < N - 1) {
        result.push_back(Move(0, 1));
    }
    if (zero_x > 0) {
        result.push_back(Move(-1, 0));
    }
    if (zero_y > 0) {
        result.push_back(Move(0, -1));
    }
    return result;
}

int Board::CalculateManhattanDistance() {
    int count = 0;
    for (auto it = tiles.begin(); it != tiles.end(); it++) {
        int tile_y = (int) (it-tiles.begin()) / N;
        int tile_x = (int) (it-tiles.begin()) - tile_y*N;
        int correct_x;
        int correct_y;
        if (*it == 0) {
            correct_x = 2;
            correct_y = 2;
        } else {
            correct_y = (*it - 1) / N;
            correct_x = *it - 1 - correct_y * N;
        }
        count += abs(tile_y - correct_y) + abs(tile_x - correct_x);
    }
    return count;
}

int main(int argc, const char * argv[]) {
    
//    freopen("puzzle.in","r",stdin);
//    freopen("puzzle.out","w",stdout);
#ifdef DEBUG
    clock_t t1 = clock();
#endif
    
    int N = 3;
    vector<int> tiles;
    for (int i = 0; i < N*N; i++) {
        int t;
        cin >> t;
        tiles.push_back(t);
    }
    shared_ptr<Solver> solver = make_shared<Solver>();
    weak_ptr<Solver> solver_weak = solver;
    shared_ptr<Board> board (new Board(tiles,N,0,solver_weak, ""));
    solver->Solve(board);
    
#ifdef DEBUG
    clock_t t2 = clock();
    cout << "Execution time: " << ((float) t2 - (float) t1)/CLOCKS_PER_SEC << endl;
#endif
    
    return 0;
}

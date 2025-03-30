#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

using namespace std;

// Some helpers ========>

#define CHAR_COUNT(s) (sizeof(s) / sizeof(char))
// Read from stdin a full line and convert it to int.
// Warning: no error checking
int read_stdin_line_as_int() {
  char line[500];
  fgets(line, CHAR_COUNT(line), stdin);
  return atoi(line);
}

// Read from stdin a full line and return the entire line with trailing '\n'
// stripped. Note: not sure on Windows, all *nix/BSD variants including Mac
// should be fine.
void read_stdin_line(char *line, int char_count) {
  fgets(line, char_count, stdin);
  int i = (int)strlen(line) - 1;
  for (; i >= 0 && line[i] == '\n'; i--) {
    line[i] = '\0';
  }
}

// For coordinates comparison (used in set operations). We use std::pair for an
// (x,y) coordinate, where pair.first is x and pair.second is y.
struct pair_hash {
  size_t operator()(const pair<int, int> &x) const {
    return x.first ^ x.second;
  }
};

// Representing a piece consisting of a number of (x,y) coordinates.
typedef vector<pair<int, int>> piece;

bool coord_compare(const pair<int, int> &a, const pair<int, int> &b) {
  // Sorted by y then x.
  return (a.second < b.second) || (a.second == b.second && a.first < b.first);
}

// After mirroring/rotation, a piece's coordinates might be shifted into
// unfavorable ranges such as < 0. Normalize them.
void normalize_piece(piece &p) {
  int min_x, min_y = 999999;  // reasonable enough? :)
  for (auto &xy : p) {
    min_x = min(min_x, xy.first);
    min_y = min(min_y, xy.second);
  }
  for (auto &xy : p) {
    xy.first -= min_x;
    xy.second -= min_y;
  }
  sort(p.begin(), p.end(), coord_compare);
}
// <========= Some helpers

// DLX node definition
struct node {
  node *left, *right, *up, *down, *col_head;
  int x, y;  // coordinates of the node
  int pid;   // the ID of the pentomino piece occupying this node.
  // TODO:  add an integer row_count for optimization so during the recursive
  // search we can always prioritize the high row count column, because covering
  // such column would "eliminate" or "cover" the most number of rows.

  node()
      : left(nullptr),
        right(nullptr),
        up(nullptr),
        down(nullptr),
        col_head(nullptr),
        x(-1),
        y(-1),
        pid(-1) {}
};

// A simple node allocation manager so when the program is done, we can free all
// nodes.
struct nodes {
  vector<node *> repo;

  ~nodes() {
    for (auto ptr : this->repo) {
      delete ptr;
    }
  }

  node *alloc() {
    auto ptr = new node();
    this->repo.push_back(ptr);
    return ptr;
  }
};

// Given a node, mark its column "covered" or "satisfied". Because the column is
// "covered", all the rows that have this column also need to be eliminated for
// future branching.
void cover_col(node *n) {
  auto col = n->col_head;
  // To "cover" the column, we re-wire the left/right neighboring columns
  // pointers so this column will be isolated and skipped in the column chain.
  // Note: we didn't touch/change this column's own left/right pointer values,
  // so that when we backtrack in the recursion, we can use them to restore the
  // column back into the column chain.
  col->right->left = col->left;
  col->left->right = col->right;
  // traverse down all the remaining rows in the column.
  for (auto r = col->down; r != col; r = r->down) {
    // traverse across all the remaining columns in the row.
    for (auto c = r->right; c != r; c = c->right) {
      // "Cover" this cell: using similar tactics described above in `col`
      // covering.
      c->down->up = c->up;
      c->up->down = c->down;
      // TODO: c->col_head->row_count--;
    }
  }
}

// Given a node, uncover the column and associated rows, the exact opposite of
// what cover_col does.
void uncover_col(node *n) {
  auto col = n->col_head;
  for (auto r = col->up; r != col; r = r->up) {
    for (auto c = r->left; c != r; c = c->left) {
      // TODO: c->col_head->row_count++;
      c->down->up = c;
      c->up->down = c;
    }
  }
  col->right->left = col;
  col->left->right = col;
}

void print_result(const node *header, vector<node *> &result) {
  static int total_solution_count = 0;

  printf("Solution #%d:\n", ++total_solution_count);

  // Remember we stored board row count in header->y and board col count in
  // header->x in main()'s initialization
  int cols = header->x;
  int rows = header->y;
  char b[rows][cols];
  memset(b, ' ', cols * rows);
  // The vector result contain all the pieces in a solution. Each piece
  // contains multiple nodes. We'll print out those pieces using their
  // respective PID onto the board.
  for (auto piece : result) {
    auto n = piece;
    do {
      if (n->x >= 0) {
        // pid, whose value is >=0 integer, to visual character mapping.
        // TODO: make this mapping more robust so we can solve problems with
        // more than 26 pieces. :)
        b[n->y][n->x] = 'A' + n->pid;
      }
      n = n->right;
    } while (n != piece);
  }
  for (int y = 0; y < header->y; y++) {
    for (int x = 0; x < header->x; x++) {
      printf("%c", b[y][x]);
    }
    printf("\n");
  }
}

void solve(const node *header, vector<node *> &result) {
  // Find the first column to cover.
  // TODO: explore different strategies to find a column to cover so the
  // branching depth is optimal.
  node *col = header->right;
  if (col == header) {
    print_result(header, result);
    return;
  }
  cover_col(col);
  // To explore how to cover this column, we'll try all the rows that offer
  // covering for this column.
  for (auto row = col->down; row != col; row = row->down) {
    // Now we've decided to use this 'row' to cover this 'col', we need to mark
    // all other columns covered by this row as "covered".
    for (auto r = row->right; r != row; r = r->right) {
      cover_col(r);
    }
    result.push_back(row);
    solve(header, result);
    result.pop_back();
    for (auto l = row->left; l != row; l = l->left) {
      uncover_col(l);
    }
  }
  uncover_col(col);
}

// General pentomino solver using DLX algo.
// Input from stdin with the following format:
//
// line 1: int N, total row number of the board.
// line 2 to N+1: followed by N lines of strings consisting of 'X's and ' 's
// (spaces). X denotes a cell that needs to be covered. Space indicates the
// position is not part of the board.
// line N+2: int M, total number of available pieces to cover the board.
// line N+3 and on: followed by M number of piece definitions. Each piece
// defintion is as follows:
//     line 1: int K, total row number of this piece.
//     line 2 to K+1: followed by K lines of strings consisting of 'X's and '
//     's. Similar to the board definition.
//
// Example stdin:
// ----------------
// 2
// XX
// XXX
// 2
// 1
// XX
// 2
// XX
// X
// ----------------
// The board has 5 cells/positions to cover. There are two pieces available to
// use: one is a 1x2 and the other is a L shaped piece.
int main() {
  nodes ns;
  node *header = ns.alloc();
  // we never use up/down ptrs on the special node header, so let them stay
  // nullptr.
  header->left = header->right = header;

  // board initialization
  unordered_set<pair<int, int>, pair_hash> board_coords;
  // store the board row count in header->y for later print_result() use.
  header->y = read_stdin_line_as_int();
  // we don't know the board width yet, need to go through all the board
  // lines to find out.
  header->x = 0;
  for (int y = 0; y < header->y; y++) {
    char line[1000];  // reasonable enough right :)
    read_stdin_line(line, CHAR_COUNT(line));
    for (int x = 0; x < strlen(line); x++) {
      if (line[x] == 'X') {
        board_coords.emplace(x, y);
        // Each board cell becomes a column in DLX's algo to be covered once and
        // exactly once.
        auto n = ns.alloc();
        n->x = x;
        n->y = y;
        n->up = n->down = n->col_head = n;
        n->left = header->left;
        n->right = header;
        header->left->right = n;
        header->left = n;
      } else if (line[x] == ' ') {
        // ' ' is acceptable, representing a hole/an empty spot on the board.
      } else {
        printf("invalid char '%c'\n", line[x]);
        exit(-1);
      }
    }
    header->x = max(header->x, (int)strlen(line));
  }

  // piece initialization
  int piece_number = read_stdin_line_as_int();
  for (int pid = 0; pid < piece_number; pid++) {
    // Each piece has its own column in the DLX's algo to be covered. So that we
    // don't use a piece more than once or not use it at all.
    auto n = ns.alloc();
    n->pid = pid;
    n->up = n->down = n->col_head = n;
    n->left = header->left;
    n->right = header;
    header->left->right = n;
    header->left = n;

    piece p;
    int lines_of_piece = read_stdin_line_as_int();
    for (int y = 0; y < lines_of_piece; y++) {
      char line[500];
      read_stdin_line(line, CHAR_COUNT(line));
      for (int x = 0; x < strlen(line); x++) {
        if (line[x] == 'X') {
          p.emplace_back(x, y);
        }
      }
    }
    normalize_piece(p);
    set<piece> variations;
    // 4 rotations x 2 mirroring
    for (int variation = 0; variation < 8; variation++) {
      // p is expected to be normalized.
      if (variations.count(p) > 0) {
        // already seen this variation before, no need to process it.
        continue;
      }
      variations.insert(p);
      // find all the positions the piece can cover.
      for (int y = 0; y < header->y; y++) {
        for (int x = 0; x < header->x; x++) {
          // Can this piece (with its mirroring/rotation) be placed at this
          // board position starting at (x,y)? Therefore we need to shift all
          // the coords in the piece by (x,y).
          piece piece_coords;
          for (auto &xy : p) {
            piece_coords.emplace_back(x + xy.first, y + xy.second);
          }
          bool fit = true;
          for (auto &xy : piece_coords) {
            if (board_coords.count(xy) <= 0) {
              fit = false;
              break;
            }
          }
          if (!fit) {
            continue;
          }

          // So this piece with its current mirroring/rotation can fit into the
          // board starting at (x,y). So we need to create a node representing
          // this piece that covers its piece column 'n'.
          auto ptr = ns.alloc();
          ptr->pid = pid;
          ptr->left = ptr->right = ptr;
          ptr->up = n->up;
          ptr->down = n;
          ptr->col_head = n;
          n->up->down = ptr;
          n->up = ptr;

          for (int i = piece_coords.size() - 1; i >= 0; i--) {
            // Then for each of the piece's coordinates, create a node to cover
            // the corresponding board cell column.
            auto &xy = piece_coords[i];
            for (auto col = header->right; col->pid == -1; col = col->right) {
              if (col->x != xy.first || col->y != xy.second) {
                continue;
              }
              auto ptr2 = ns.alloc();
              ptr2->x = col->x;
              ptr2->y = col->y;
              ptr2->pid = pid;
              ptr2->left = ptr;
              ptr2->right = ptr->right;
              ptr2->up = col->up;
              ptr2->down = col;
              ptr2->col_head = col;
              // Link each of the piece coordinate nodes back to the piece node.
              // So this row in DLX is entirely about this piece,
              ptr->right->left = ptr2;
              ptr->right = ptr2;
              col->up->down = ptr2;
              col->up = ptr2;
              break;
            }
          }
        }
      }
      // rotate or mirror:
      // variation loops from 0 to 7. For 0, 1, 2 let's rotate 90deg
      // counter-clockwise. on 3 let's mirror. then 4, 5, 6, rotate 90deg CCW.
      // On 7 we'll rotate again, but doesn't matter as the loop ends.
      for (auto &xy : p) {
        int cur_x = xy.first, cur_y = xy.second;
        if (variation != 3) {
          // rotate
          xy.first = cur_y;
          xy.second = -cur_x;
        } else {
          // mirror
          xy.first = -cur_x;
          xy.second = cur_y;
        }
      }
      normalize_piece(p);
    }
  }
  vector<node *> result;
  solve(header, result);
  return 0;
}

#include <iostream>
#include <chrono>
#include <sstream>

std::chrono::time_point<std::chrono::system_clock> start_time;


void tic() {
  start_time = std::chrono::system_clock::now();
}

int toc() {
  std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = current_time - start_time;
  return int(elapsed.count()*1000.0);
}

enum e_playertype {
  empty = 0,
  beginner = 1,
  other=2
};

inline e_playertype other_player(e_playertype pt) {
  return (e_playertype)(pt ^ 3);
}

inline int get_index(int row, int col) {
  return row*3+col;
}

inline int player_mask(int index, e_playertype pt) {
  return (int)pt << (2*index);
}

inline int player_mask(int row, int col, e_playertype pt) {
  return player_mask(get_index(row, col), pt);
}

inline int put_player(int index, e_playertype pt, int board) {
  return board | player_mask(index, pt);
}

inline int put_player(int row, int col, e_playertype pt, int board) {
  return put_player(get_index(row, col), pt, board);
}

inline e_playertype get_player(int index, int board) {
  return (e_playertype)((board >> (2*index))&3);
}

inline e_playertype get_player(int row, int col, int board) {
  return get_player(get_index(row, col), board);
}

e_playertype get_winner(int board) {
  const e_playertype p00 = (e_playertype)(board&3);
  const e_playertype p01 = (e_playertype)((board>>2)&3);
  const e_playertype p02 = (e_playertype)((board>>4)&3);
  const e_playertype p10 = (e_playertype)((board>>6)&3);
  const e_playertype p11 = (e_playertype)((board>>8)&3);
  const e_playertype p12 = (e_playertype)((board>>10)&3);
  const e_playertype p20 = (e_playertype)((board>>12)&3);
  const e_playertype p21 = (e_playertype)((board>>14)&3);
  const e_playertype p22 = (e_playertype)((board>>16)&3);
  if (p00) {
    if (p00 == p01 && p00 == p02)
      return p00;
    if (p00 == p10 && p00 == p20)
      return p00;
    if (p00 == p11 && p00 == p22)
      return p00;
  }
  if (p11) {
    if (p11 == p10 && p11 == p12)
      return p11;
    if (p01 == p11 && p21 == p11)
      return p11;
    if (p02 == p11 && p20 == p11)
      return p11;
  }
  if (p22) {
    if (p02 == p22 && p12 == p22)
      return p22;
    if (p20 == p22 && p21 == p22)
      return p22;
  }
  return e_playertype::empty;
}

#define max_table_size 262144

void fill_table(int& gametree_size, int& number_of_games_won_by_beginner, int& number_of_games_won_by_other, float* beginner_table, float* other_table, int board, e_playertype player, int depth) {
  e_playertype winner = get_winner(board);
  if (winner != e_playertype::empty) {
    gametree_size = 1;
    number_of_games_won_by_beginner = winner == e_playertype::beginner;
    number_of_games_won_by_other = winner == e_playertype::other;
    return;
  }
  if (depth == 0) {
    gametree_size = 1;
    number_of_games_won_by_beginner = 0;
    number_of_games_won_by_other = 0;
    return;
  }
  gametree_size = 0;
  number_of_games_won_by_beginner = 0;
  number_of_games_won_by_other = 0;
  auto swapped_player = other_player(player);
  for (int i = 0; i < 9; ++i) {
    if (get_player(i, board) == e_playertype::empty) {
      int new_board = put_player(i, player, board);
      int rec_gametree_size, rec_number_of_games_won_by_beginner, rec_number_of_games_won_by_other;
      fill_table(rec_gametree_size, rec_number_of_games_won_by_beginner, rec_number_of_games_won_by_other, beginner_table, other_table, new_board, swapped_player, depth-1);
      beginner_table[new_board] = (double)rec_number_of_games_won_by_beginner/(double)rec_gametree_size;
      other_table[new_board] = (double)rec_number_of_games_won_by_other/(double)rec_gametree_size;
      gametree_size += rec_gametree_size;
      number_of_games_won_by_beginner += rec_number_of_games_won_by_beginner;
      number_of_games_won_by_other += rec_number_of_games_won_by_other;
    }
  }
}

// this table returns the percentage of wins for the beginner for a given board situation
void build_tables(float*& beginner_table, float*& other_table) {
  beginner_table = new float[max_table_size];
  other_table = new float[max_table_size];
  for (int i = 0; i < max_table_size; ++i)
    beginner_table[i] = -1.f; // init everything to invalid
  for (int i = 0; i < max_table_size; ++i)
    other_table[i] = -1.f; // init everything to invalid
  
  int total_gametree_size = 0;
  int total_number_of_games_won_by_beginner = 0;
  int total_number_of_games_won_by_other = 0;
  
  for (int i = 0; i < 9; ++i) {
    int board = put_player(i, e_playertype::beginner, 0);
    int gametree_size, number_of_games_won_by_beginner, number_of_games_won_by_other;
    fill_table(gametree_size, number_of_games_won_by_beginner, number_of_games_won_by_other, beginner_table, other_table, board, e_playertype::other, 8);
    beginner_table[board] = (double)number_of_games_won_by_beginner/(double)gametree_size;
    other_table[board] = (double)number_of_games_won_by_other/(double)gametree_size;
    total_gametree_size += gametree_size;
    total_number_of_games_won_by_beginner += number_of_games_won_by_beginner;
    total_number_of_games_won_by_other += number_of_games_won_by_other;
  }
  beginner_table[0] = (double)total_number_of_games_won_by_beginner/(double)total_gametree_size;
  other_table[0] = (double)total_number_of_games_won_by_other/(double)total_gametree_size;
}

void print_board(int board) {
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      auto pt = get_player(r, c, board);
      switch (pt) {
        case e_playertype::empty: std::cout << " . "; break;
        case e_playertype::beginner: std::cout << " x "; break;
        case e_playertype::other: std::cout << " o "; break;
      }
    }
    std::cout << std::endl;
  }
}

void print_table_result(int board, float* beginner_table, float* other_table) {
  std::cout << "Percentage of wins for beginner: " << beginner_table[board]*100.f << "%" << std::endl;
  std::cout << "Percentage of wins for other: " << other_table[board]*100.f << "%" << std::endl;
}

void print_result(int board) {
  auto winner = get_winner(board);
  if (winner == e_playertype::beginner) {
    std::cout << "Beginner wins!" << std::endl;
    return;
  } else if (winner == e_playertype::other) {
    std::cout << "Other player wins!" << std::endl;
    return;
  }
  int no_legal_moves = true;
  for (int i = 0; i < 9; ++i) {
    if (get_player(i, board) == e_playertype::empty) {
      no_legal_moves = false;
      break;
    }
  }
  if (no_legal_moves)
    std::cout << "Draw!" << std::endl;
}

void think_1level(int& index, double& chance_of_winning, int board, e_playertype computer_side, float* beginner_table, float* other_table) {
  index = -1;
  chance_of_winning = -1.0;
  for (int i = 0; i < 9; ++i) {
    if (get_player(i, board) == e_playertype::empty) {
      int new_board = put_player(i, computer_side, board);
      double score = 0.f;
      if (computer_side == e_playertype::beginner)
        score = beginner_table[new_board];
      else if (computer_side == e_playertype::other)
        score = other_table[new_board];
      if (score > chance_of_winning) {
        chance_of_winning = score;
        index = i;
      }
    }
  }
}

void think(int& index, double& chance_of_winning, int board, e_playertype computer_side, float* beginner_table, float* other_table) {
  index = -1;
  chance_of_winning = -1.0;
  for (int i = 0; i < 9; ++i) {
    if (get_player(i, board) == e_playertype::empty) {
      int new_board = put_player(i, computer_side, board);
      double score = 0.f;
      if (computer_side == e_playertype::beginner) {
        score = beginner_table[new_board];
        if (!score)
          score = 0.00001f;
        int tmp; double multiplier;
        think_1level(tmp, multiplier, new_board, e_playertype::other, beginner_table, other_table);
        score *= (1.0-multiplier);
        }
      else if (computer_side == e_playertype::other) {
        score = other_table[new_board];
        if (!score)
          score = 0.001f;
        int tmp; double multiplier;
        think_1level(tmp, multiplier, new_board, e_playertype::beginner, beginner_table, other_table);
        score *= (1.0-multiplier);
        }
      if (score > chance_of_winning) {
        chance_of_winning = score;
        index = i;
      }
    }
  }
}

int main() {
  tic();
  float* beginner_table;
  float* other_table;
  build_tables(beginner_table, other_table);
  int time_spent = toc();
  
  std::cout << "TicTacToe" << std::endl;
  std::cout << "=========" << std::endl;
  std::cout << "Building the table took " << time_spent << "ms" << std::endl;
  
  std::cout << "Type 2 seperated integers 'row' and 'col' to put a marker on the board." << std::endl;
  std::cout << "Type new to begin a new game." << std::endl;
  std::cout << "Type d to print the board." << std::endl;
  std::cout << "Type go to let the computer play." << std::endl;
  std::cout << "Type hint to get the best current move." << std::endl;
  std::cout << "Type exit or quit to quit the game." << std::endl;
  e_playertype computer_side = e_playertype::empty;
  e_playertype side_to_move = e_playertype::beginner;
  int board = 0;
  char command_line[256], command[256];
  for (;;) {
    if (side_to_move == computer_side) {
      int index;
      double chance_of_winning;
      think(index, chance_of_winning, board, computer_side, beginner_table, other_table);
      if (index < 0)
        {
        computer_side = e_playertype::empty;
        continue;
        }
      board = put_player(index, side_to_move, board);
      print_board(board);
      print_table_result(board, beginner_table, other_table);
      print_result(board);
      side_to_move = other_player(side_to_move);
      continue;
    }
    std::cout << ":0> ";
    if (!fgets(command_line, 256, stdin))
    {
      std::cout << "Error: cannot read from stdin\n";
      delete [] beginner_table;
      delete [] other_table;
      return -1;
    }
    if (command_line[0] == '\n')
      continue;
    sscanf(command_line, "%s", command);
    if (std::string(command) == std::string("go"))
    {
      computer_side = side_to_move;
      continue;
    }
    if (std::string(command) == std::string("d"))
    {
      print_board(board);
      print_table_result(board, beginner_table, other_table);
      print_result(board);
      fflush(stdout);
      continue;
    }
    if (std::string(command) == std::string("new"))
    {
      computer_side = e_playertype::empty;
      board = 0;
      continue;
    }
    if (std::string(command) == std::string("exit"))
    {
      break;
    }
    if (std::string(command) == std::string("quit"))
    {
      break;
    }
    if (std::string(command) == std::string("hint"))
    {
    int index;
    double chance_of_winning;
    think(index, chance_of_winning, board, side_to_move, beginner_table, other_table);
    std::cout << "Hint: " << index/3 << " " << index%3 << std::endl;
    std::cout << "Chance of winning: " << chance_of_winning*100.f << "%" << std::endl;
    continue;
    }
    std::stringstream str;
    str << std::string(command_line);
    int row, col;
    str >> row >> col;
    if (row < 0 || col < 0 || row >= 3 || col >= 3 || get_player(row, col, board) != e_playertype::empty) {
      std::cout << "Illegal move" << std::endl;
    }
    else {
      board = put_player(row, col, side_to_move, board);
      print_board(board);
      print_table_result(board, beginner_table, other_table);
      print_result(board);
      side_to_move = other_player(side_to_move);
    }
  }
  
  delete [] beginner_table;
  delete [] other_table;
  return 0;
}

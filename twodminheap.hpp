#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

class TwoDMinHeap {
 private:
  int n;
  int heap_size;
  int table_size;
  int table_num;
  int table_log;
  vector<int> data;
  vector<int> data_to_heap;
  vector<int> heap;
  vector<int> heap_to_data;
  vector<int> parent;

  // for sparse table
  vector<int> min_table;
  vector<int> pos_table;
  vector<vector<int> > sparse_table;

  // for head and tail
  vector<int> table_type;
  vector<vector<vector<int> > > table_RMQ;

 public:
  TwoDMinHeap();
  TwoDMinHeap(int l);

  void addNumber(int x);
  int RMQ(int i, int j);
  void test();

 private:
  void calcTableRMQ();
  void updateSparseTable();
  int sparseTableRMQ(int i, int j);
};

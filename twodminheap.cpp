#include "twodminheap.hpp"

TwoDMinHeap::TwoDMinHeap() {}

TwoDMinHeap::TwoDMinHeap(int l) {
  n = l;
  heap_size = 2 * n;
  table_size = (int)ceil(log2((double)heap_size) / 2.0);
  table_num = (heap_size + table_size - 1) / table_size;
  table_log = (int)log2((double)table_num) + 1;
  sparse_table.resize(table_num, vector<int>(table_log, 0));
  data.push_back(-1);
  heap.push_back(0);
  data_to_heap.push_back(0);
  heap_to_data.push_back(0);
  parent.push_back(0);
  table_type.resize(table_num);
  calcTableRMQ();
}

void TwoDMinHeap::addNumber(int x) {
  int index = data.size() - 1;
  data.push_back(x);
  while (x <= data[index]) {
    index = parent[index];
    heap.push_back(heap[heap.size() - 1] - 1);
    heap_to_data.push_back(index);
    if ((heap.size() - 1) % table_size == 0) {
      updateSparseTable();
    }
  }
  heap.push_back(heap[heap.size() - 1] + 1);
  heap_to_data.push_back(data.size() - 1);
  data_to_heap.push_back(heap.size() - 1);
  table_type[(heap.size() - 2) / table_size] |=
      1 << (table_size - ((heap.size() - 2) % table_size) - 1);
  parent.push_back(index);
  if ((heap.size() - 1) % table_size == 0) {
    updateSparseTable();
  }

  return;
}

int TwoDMinHeap::RMQ(int i, int j) {
  i++;
  j++;
  int i_heap = data_to_heap[i] - 1;
  int j_heap = data_to_heap[j] - 1;
  int i_table = i_heap / table_size;
  int j_table = j_heap / table_size;
  int i_pos = i_heap % table_size;
  int j_pos = j_heap % table_size;
  int heap_pos;
  if (i_table == j_table) {
    heap_pos =
        1 + i_table * table_size + table_RMQ[table_type[i_table]][i_pos][j_pos];
  } else if (j_table - i_table == 1) {
    if (heap[1 + i_table * table_size +
             table_RMQ[table_type[i_table]][i_pos][table_size - 1]] <
        heap[1 + j_table * table_size +
             table_RMQ[table_type[j_table]][0][j_pos]]) {
      heap_pos = 1 + i_table * table_size +
                 table_RMQ[table_type[i_table]][i_pos][table_size - 1];
    } else {
      heap_pos =
          1 + j_table * table_size + table_RMQ[table_type[j_table]][0][j_pos];
    }
  } else {
    if (heap[1 + i_table * table_size +
             table_RMQ[table_type[i_table]][i_pos][table_size - 1]] <
            heap[sparseTableRMQ(i_table + 1, j_table - 1)] &&
        heap[1 + i_table * table_size +
             table_RMQ[table_type[i_table]][i_pos][table_size - 1]] <
            heap[1 + j_table * table_size +
                 table_RMQ[table_type[j_table]][0][j_pos]]) {
      heap_pos = 1 + i_table * table_size +
                 table_RMQ[table_type[i_table]][i_pos][table_size - 1];
    } else if (heap[sparseTableRMQ(i_table + 1, j_table - 1)] <
               heap[1 + j_table * table_size +
                    table_RMQ[table_type[j_table]][0][j_pos]]) {
      heap_pos = sparseTableRMQ(i_table + 1, j_table - 1);
    } else {
      heap_pos =
          1 + j_table * table_size + table_RMQ[table_type[j_table]][0][j_pos];
    }
  }
  int data_pos = heap_to_data[heap_pos];
  if (data[i] == data[data_pos]) {
    return data[i];
  } else {
    return data[heap_to_data[heap_pos + 1]];
  }
}

void TwoDMinHeap::test() {
  cout << "parameter" << endl;
  cout << n << " " << table_size << " " << table_num << " " << table_log
       << endl;
  cout << "data" << endl;
  for (size_t i = 1; i < data.size(); i++) {
    cout << data[i];
    i == data.size() - 1 ? cout << endl : cout << " ";
  }
  for (size_t i = 1; i < data_to_heap.size(); i++) {
    cout << data_to_heap[i];
    i == data_to_heap.size() - 1 ? cout << endl : cout << " ";
  }
  for (size_t i = 1; i < parent.size(); i++) {
    cout << parent[i];
    i == parent.size() - 1 ? cout << endl : cout << " ";
  }
  cout << "heap" << endl;
  for (size_t i = 0; i < heap.size(); i++) {
    cout << heap[i];
    i == heap.size() - 1 ? cout << endl : cout << " ";
  }
  for (size_t i = 0; i < heap_to_data.size(); i++) {
    cout << heap_to_data[i];
    i == heap_to_data.size() - 1 ? cout << endl : cout << " ";
  }
  cout << "sparse table" << endl;
  for (size_t i = 0; i < min_table.size(); i++) {
    cout << min_table[i];
    i == min_table.size() - 1 ? cout << endl : cout << " ";
  }
  for (size_t i = 0; i < pos_table.size(); i++) {
    cout << pos_table[i];
    i == pos_table.size() - 1 ? cout << endl : cout << " ";
  }
  for (int i = 0; i < table_num; i++) {
    for (int j = 0; j < table_log; j++) {
      cout << i << " " << j << " " << sparse_table[i][j] << endl;
    }
  }
  cout << "sparse table RMQ" << endl;
  for (size_t i = 0; i < min_table.size(); i++) {
    for (size_t j = i; j < min_table.size(); j++) {
      cout << i << " " << j << " " << sparseTableRMQ(i, j) << endl;
    }
  }
  cout << "RMQ" << endl;
  for (size_t i = 0; i + 1 < data.size(); i++) {
    for (size_t j = i; j + 1 < data.size(); j++) {
      cout << i << " " << j << " " << RMQ(i, j) << endl;
    }
  }
  return;
}

void TwoDMinHeap::calcTableRMQ() {
  table_RMQ.resize(1 << table_size,
                   vector<vector<int> >(table_size, vector<int>(table_size)));
  for (int i = 0; i < (1 << table_size); i++) {
    for (int l = 0; l < table_size; l++) {
      int min_pos = l;
      int minim = 0;
      int val = 0;
      for (int r = 0; r < table_size; r++) {
        if (r < l) {
          table_RMQ[i][l][r] = -1;
        } else if (r == l) {
          table_RMQ[i][l][r] = l;
        } else {
          if (i & (1 << (table_size - r - 1))) {
            val++;
          } else {
            val--;
            if (val <= minim) {
              minim = val;
              min_pos = r;
            }
          }
          table_RMQ[i][l][r] = min_pos;
        }
      }
    }
  }
}

void TwoDMinHeap::updateSparseTable() {
  int min_pos = heap.size() - table_size;
  for (size_t i = heap.size() - table_size + 1; i < heap.size(); i++) {
    if (heap[i] <= heap[min_pos]) {
      min_pos = i;
    }
  }
  min_table.push_back(heap[min_pos]);
  pos_table.push_back(min_pos);
  sparse_table[min_table.size() - 1][0] = pos_table[min_table.size() - 1];

  for (int i = 1; (1 << i) - 1 < min_table.size(); i++) {
    if (heap[sparse_table[min_table.size() - (1 << i)][i - 1]] <
        heap[sparse_table[min_table.size() - (1 << (i - 1))][i - 1]]) {
      sparse_table[min_table.size() - (1 << i)][i] =
          sparse_table[min_table.size() - (1 << i)][i - 1];
    } else {
      sparse_table[min_table.size() - (1 << i)][i] =
          sparse_table[min_table.size() - (1 << (i - 1))][i - 1];
    }
  }
}

int TwoDMinHeap::sparseTableRMQ(int i, int j) {
  int k = (int)log2(j - i + 1);
  if (heap[sparse_table[i][k]] < heap[sparse_table[j - (1 << k) + 1][k]]) {
    return sparse_table[i][k];
  } else {
    return sparse_table[j - (1 << k) + 1][k];
  }
}

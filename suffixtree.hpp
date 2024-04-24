#pragma once

#ifndef __SUFFIXTREE_H__

#define __SUFFIXTREE_H__

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <vector>

using namespace std;
#include "lzhb_common.hpp"
class UkkonenSTree {
 private:
  struct Node;

  struct Edge {
    uInt start;
    uInt end;
    Node* child;
    Edge(){};
    Edge(uInt s, uInt e, Node* n) : start(s), end(e), child(n) {}
    ~Edge() { delete child; }
  };

  struct Node {
    map<char, Edge*> edges;
    Node* slink;
    Edge* parent;
    vector<uInt> pos;
    Node() { slink = NULL; }
    ~Node() {
      for (auto it = edges.begin(); it != edges.end(); it++) {
        delete it->second;
      }
    }
  };

 public:
  class Point {
    Node* node;  // source node
    Edge* edge;  // outgoing edge. NULL if at source node
    uInt l;      // length from start of outgoing edge, 0 if at node

   public:
    Point(Node* nd, Edge* eg, uInt c) : node(nd), edge(eg), l(c) {}
    friend class UkkonenSTree;
  };

 private:
  string T;
  Node* root;
  Edge* aux;
  Point active_point;
  Node* start_leaf;
  Node* last_leaf;
  int64_t next_leaf;
  Node* split(Node* parent, Node* child, char c);
  Point readEdge(Point ap, char c) const;
  void addSuffix(uInt pos);

 public:
  // init tree
  UkkonenSTree();
  // init tree and construct the suffix tree of t
  UkkonenSTree(const string& t);
  // append t to T
  void addString(const string& t);

  // append char c to T
  void addChar(char c);

  // assuming T end and finalize the suffix tree
  void terminal();

  // get locus representation of root
  Point get_root() const { return Point(root, NULL, 0); }

  // traverse with char c from point pt. return new point and last position
  pair<Point, uInt> traverse(Point pt, char c) const;

  // find the longest prefix of p that occurs in T, returns (length, starting
  // position)
  pair<uInt, uInt> matching(const string& p) const;
  // find the longest extension of p[start..] that occurs in T, returns
  // (length, starting position)
  pair<uInt, uInt> matching(const string& p, uInt start) const;
  pair<uInt, uInt> matching(const string& p, uInt start, uInt end) const;

  // returns occurrence positions of p in R.
  std::vector<uInt> findOccurrences(const string& p, uInt start,
                                    uInt len) const;

  // show the current suffix tree
  void printTree() const;

  static const Point voidPoint;
  ~UkkonenSTree();
};

#endif  // __SUFFIXTREE_H__
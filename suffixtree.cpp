#include "suffixtree.hpp"

// const UkkonenSTree::Point UkkonenSTree::voidPoint(NULL, NULL, 0);

UkkonenSTree::UkkonenSTree()
    : root(new Node), aux(new Edge(0, 0, root)), active_point(root, NULL, 0) {
  next_leaf = 0;
  last_leaf = NULL;
  start_leaf = NULL;
}

UkkonenSTree::UkkonenSTree(const string& t)
    : root(new Node), aux(new Edge(0, 0, root)), active_point(root, NULL, 0) {
  next_leaf = 0;
  last_leaf = NULL;
  start_leaf = NULL;
  T = t;
  addSuffix(0);
}

UkkonenSTree::Point UkkonenSTree::readEdge(Point ap, char c) const {
  if (ap.edge != NULL) {
    if (ap.edge->start + ap.l >= T.size()) {
      return Point(NULL, NULL, 0);
    }
    if (T[ap.edge->start + ap.l] == c) {
      // cout << "edge exists" << endl;
      if (ap.edge->start + ap.l == ap.edge->end) {
        return Point(ap.edge->child, NULL, 0);
      } else {
        ap.l++;
        return ap;
      }
    } else {
      // no path labeled by c
      return Point(NULL, NULL, 0);
    }
  } else {
    if (ap.node->edges.find(c) != ap.node->edges.end()) {
      auto eg = ap.node->edges[c];
      if (eg->start == eg->end) {
        return Point(eg->child, NULL, 0);
      } else {
        return Point(ap.node, eg, 1);
      }
    } else {
      // no path labeled by c
      return Point(NULL, NULL, 0);
    }
  }
}

void UkkonenSTree::addSuffix(uInt pos) {
  for (uInt i = pos; i < T.size(); i++) {
    char c = T[i];
    Point next_ap = readEdge(active_point, c);
    Node* last_node = NULL;
    while (next_ap.node == NULL) {
      if (active_point.edge == NULL) {
        Node* new_leaf = new Node;
        Edge* new_edge =
            new Edge(i, std::numeric_limits<uInt>::max(), new_leaf);
        active_point.node->edges[c] = new_edge;
        if (last_node != NULL) {
          last_node->slink = active_point.node;
        }
        last_node = NULL;

        new_leaf->parent = new_edge;
        if (last_leaf == NULL) {
          start_leaf = new_leaf;

        } else {
          last_leaf->slink = new_leaf;
        }
        new_leaf->pos.push_back(next_leaf);
        next_leaf++;
        last_leaf = new_leaf;

        if (active_point.node == root) {
          next_ap = active_point;
        } else {
          active_point.node = active_point.node->slink;
          next_ap = readEdge(active_point, c);
        }
      } else {
        Node* new_leaf = new Node;
        Node* new_node = new Node;
        Edge* new_edge_to_leaf =
            new Edge(i, std::numeric_limits<uInt>::max(), new_leaf);
        Edge* new_edge_to_node =
            new Edge(active_point.edge->start,
                     active_point.edge->start + active_point.l - 1, new_node);
        uInt depth = active_point.l;

        new_node->edges[c] = new_edge_to_leaf;
        new_node->edges[T[active_point.edge->start + active_point.l]] =
            active_point.edge;
        new_node->edges[T[active_point.edge->start + active_point.l]]->start =
            active_point.edge->start + active_point.l;
        new_node->parent = new_edge_to_node;

        active_point.node->edges[T[new_edge_to_node->start]] = new_edge_to_node;
        active_point.edge = new_edge_to_node;

        if (last_node != NULL) {
          last_node->slink = new_node;
        }
        last_node = new_node;

        new_leaf->parent = new_edge_to_leaf;
        if (last_leaf == NULL) {
          start_leaf = new_leaf;
        } else {
          last_leaf->slink = new_leaf;
        }
        new_leaf->pos.push_back(next_leaf);
        next_leaf++;
        last_leaf = new_leaf;

        Node* temp;
        if (active_point.node == root) {
          depth--;
          temp = root;
        } else {
          temp = active_point.node->slink;
        }

        char d = T[active_point.edge->start + active_point.l - depth];
        while (temp->edges[d]->end - temp->edges[d]->start < depth) {
          depth -= temp->edges[d]->end - temp->edges[d]->start + 1;
          temp = temp->edges[d]->child;
          d = T[active_point.edge->start + active_point.l - depth];
        }

        active_point.node = temp;
        if (depth == 0) {
          active_point.edge = NULL;
        } else {
          active_point.edge = temp->edges[d];
        }

        active_point.l = depth;
        next_ap = readEdge(active_point, c);
      }
    }
    if (last_node != NULL) {
      last_node->slink = active_point.node;
    }

    active_point = next_ap;
  }
}

void UkkonenSTree::addChar(char c) {
  T.push_back(c);
  addSuffix(T.size() - 1);
}

void UkkonenSTree::addString(const string& t) {
  T += t;
  addSuffix(T.size() - t.size());
}

void UkkonenSTree::terminal() {
  T += "$";
  Node* last_node = NULL;
  Node* leaf = start_leaf;
  while (leaf != NULL) {
    leaf->parent->end = T.size() - 2;
    last_node = leaf;
    leaf = leaf->slink;
  }

  while (active_point.edge != NULL) {
    Node* new_node = new Node;
    Edge* new_edge_to_node =
        new Edge(active_point.edge->start,
                 active_point.edge->start + active_point.l - 1, new_node);
    uInt depth = active_point.l;

    // split edge and add leaf
    new_node->edges[T[active_point.edge->start + active_point.l]] =
        active_point.edge;
    new_node->edges[T[active_point.edge->start + active_point.l]]->start =
        active_point.edge->start + active_point.l;
    new_node->parent = new_edge_to_node;

    active_point.node->edges[T[new_edge_to_node->start]] = new_edge_to_node;
    active_point.edge = new_edge_to_node;

    if (last_node != NULL) {
      last_node->slink = new_node;
    }
    last_node = new_node;
    new_node->pos.push_back(next_leaf);
    next_leaf++;

    // next_active_point
    Node* temp;
    if (active_point.node == root) {
      depth--;
      temp = root;
    } else {
      temp = active_point.node->slink;
    }

    char d = T[active_point.edge->start + active_point.l - depth];
    while (temp->edges[d]->end - temp->edges[d]->start < depth) {
      depth -= temp->edges[d]->end - temp->edges[d]->start + 1;
      temp = temp->edges[d]->child;
      d = T[active_point.edge->start + active_point.l - depth];
    }

    active_point.node = temp;
    if (depth == 0) {
      active_point.edge = NULL;
    } else {
      active_point.edge = temp->edges[d];
    }

    active_point.l = depth;
  }

  if (last_node != NULL) {
    last_node->slink = active_point.node;
  }

  while (active_point.node != root) {
    active_point.node->pos.push_back(next_leaf);
    next_leaf++;
    active_point.node = active_point.node->slink;
  }
  active_point.node = root;
  next_leaf++;
  start_leaf = NULL;
  last_leaf = NULL;
}

pair<UkkonenSTree::Point, uInt> UkkonenSTree::traverse(UkkonenSTree::Point pt,
                                                       char c) const {
  if (pt.node == NULL)
    return make_pair(Point(NULL, NULL, 0), std::string::npos);
  size_t pos;
  Point next_ap = readEdge(pt, c);
  if (next_ap.node == NULL) {
    return make_pair(next_ap, std::string::npos);
  }
  if (pt.edge != NULL)
    pos = pt.edge->start + pt.l;
  else
    pos = pt.node->edges[c]->start;
  return make_pair(next_ap, pos);
}

pair<uInt, uInt> UkkonenSTree::matching(const string& p) const {
  pair<uInt, uInt> ans = pair<size_t, size_t>(0, 0);
  Point pt(root, NULL, 0);
  for (size_t i = 0; i < p.size(); i++) {
    char c = p[i];
    Point next_ap = readEdge(pt, c);
    if (next_ap.node == NULL) {
      return ans;
    }
    ans.first++;
    if (pt.edge != NULL) {
      ans.second = pt.edge->start + pt.l;
    } else {
      ans.second = pt.node->edges[c]->start;
    }
    pt = next_ap;
  }
  return ans;
}

pair<uInt, uInt> UkkonenSTree::matching(const string& p, uInt start) const {
  return matching(p, start, p.size());
}

pair<uInt, uInt> UkkonenSTree::matching(const string& p, uInt start,
                                        uInt end) const {
  pair<uInt, uInt> ans = pair<uInt, uInt>(0, 0);
  Point pt(root, NULL, 0);
  for (uInt i = start; i < end; i++) {
    char c = p[i];
    Point next_ap = readEdge(pt, c);
    if (next_ap.node == NULL) {
      return ans;
    }
    ans.first++;
    if (pt.edge != NULL) {
      ans.second = pt.edge->start + pt.l;
    } else {
      ans.second = pt.node->edges[c]->start;
    }
    pt = next_ap;
  }
  return ans;
}

std::vector<uInt> UkkonenSTree::findOccurrences(const string& p, uInt start,
                                                uInt len) const {
  vector<uInt> ans;
  Point pt(root, NULL, 0);
  uInt plen = std::min((uInt)p.size(), start + len) - start;
  for (uInt i = start; i < start + plen; i++) {
    char c = p[i];
    Point next_ap = readEdge(pt, c);
    if (next_ap.node == NULL) {
      return ans;
    }
    pt = next_ap;
  }

  Node* match_node;
  if (pt.edge != NULL) {
    match_node = pt.edge->child;
  } else {
    match_node = pt.node;
  }

  uInt suff_end = 0;
  uInt suff_len = T.size() - next_leaf;
  if (active_point.edge == NULL) {
    if (active_point.node != root) {
      suff_end = std::min(active_point.node->parent->end, (uInt)T.size() - 1);
    }
  } else {
    suff_end = active_point.edge->start + active_point.l - 1;
  }
  uInt suff_start = (suff_end + 1) - suff_len;

  std::stack<Node*> st;
  st.push(match_node);
  while (!st.empty()) {
    Node* cur_node = st.top();
    st.pop();
    for (uInt i = 0; i < cur_node->pos.size(); i++) {
      ans.push_back(cur_node->pos[i]);
      if (cur_node->pos[i] >= suff_start &&
          suff_end >= cur_node->pos[i] + plen - 1) {
        ans.push_back(next_leaf + (cur_node->pos[i] - suff_start));
      }
    }
    for (auto it = cur_node->edges.begin(); it != cur_node->edges.end(); it++) {
      st.push(it->second->child);
    }
  }
  return ans;
}

void UkkonenSTree::printTree() const {
  stack<Node*> st;
  st.push(root);
  while (st.size()) {
    Node* node = st.top();
    st.pop();
    cout << node << ":" << endl;
    for (auto it = node->edges.begin(); it != node->edges.end(); it++) {
      cout << +it->first << ":";
      if (it->second->end != std::numeric_limits<uInt>::max()) {
        for (uInt i = it->second->start; i <= it->second->end; i++) {
          cout << +T[i];
        }
        cout << " " << it->second->child;
      } else {
        for (uInt i = it->second->start; i < T.size(); i++) {
          cout << +T[i];
        }
        cout << " " << it->second->child;
      }
      st.push(it->second->child);
      cout << endl;
    }
    cout << endl;
  }
}

UkkonenSTree::~UkkonenSTree() {
  // This is correct, but too slow...
  // delete aux;
}
# LZHB: Height Bounded Lempel-Ziv encodings

## Build:

Requirements:
- a recent version of cmake
- c++ compiler that can handle c++20

      $ cmake -S . -B build
      $ cmake --build build

This makes executables in the directory `build/`.

## What does what

### lzhb3

Greedy lzhb encoding where each phrase starting at position $i$ is the longest $\ell$ such that there exists an occurrence of $T[i..i+\ell)$ that satisfies the height bound.

```bash
Usage:
  lzhb3_main [OPTION...]

  -f, --file arg     input file (use '-' for stdin) (default: -)
  -a, --appendchar   the phrase is the shortest prefix that has no 
                     occurrence satisfying height constraint
  -s, --suffixarray  O(n \log n) suffix array version (default is to use 
                     O(n \log \sigma) suffix tree version)
  -z, --optimize     greedily find best occurrence (minimize maximum 
                     height)
  -b, --hbound arg   maximum allowed height (default: 0xffffffffffffffff)
  -g, --verify       verify output in various ways
  -h, --help         Print usage
```

### lzhb4

Greedy lzhb encoding where each phrase starting at position $i$ is the longest $\ell$ such that there exists a reference of $T[i..i+\ell)$ that satisfies the height bound. Here, a reference of a factor is a previous occurrence of a prefix periodicity of the factor, i.e., $x$ such that $T[i..i+\ell) =x^e$ for some $e \in \{ \frac{i}{|x|} \mid i \in \{1,2,\ldots\}\}$

```bash
Usage:
  lzhb4_main [OPTION...]

  -f, --file arg     input file (use '-' for stdin) (default: -)
  -a, --appendchar   the phrase is the shortest prefix that has no 
                     occurrence satisfying height constraint
  -s, --suffixarray  O(n \log n) suffix array version (default is to use 
                     O(n \log \sigma) suffix tree version)
  -z, --optimize     greedily find best occurrence (minimize maximum 
                     height)
  -b, --hbound arg   maximum allowed height (default: 0xffffffffffffffff)
  -g, --verify       verify output in various ways
  -h, --help         Print usage
```

The programs only output a summary of the parsing in a comma separated format. The fields are:

1. time stamp
2. algorithm
3. filename
4. height bound
5. file size
6. parse size
7. maximum height
8. average height
9. variance of height
10. msec: running time (measured from after reading the input into memory, to finish computing of the parse) in milliseconds
11. ru_maxrss: max resident set size obtained by getrusage (bytes or kilobytes, depending on operating system)

When no filename is supplied, the programs process and outputs a summary of the parsing for each line of the standard input.

## Notes

In lzhb3, references for each position in a phrase are defined straight-fowardly from the references of the phrase, with an exception in self-referencing phrases: for a phrase $T[i..i+\ell)$ that references position $T[j+\ell)$,
the references of each position is translated to positions between $j$ and $i$ based on periodicity: i.e., for $0\leq k <\ell$, $T[i+k]$ references $T[j+(k\bmod(j-i))]$.

In lzhb4, a phrase is allowed to be a power of a prefix, i.e., $T[i..i+\ell) = x^e$ can reference a previous occurrence $j$ of $x$, i.e., $T[i+k]$ for $0 \leq k < \ell$ references $T[j+(k\bmod|x|)]$.
When $|x| = 1$, $x^k$ can be represented as a single phrase. Therefore, lzhb4 with height bound $0$ is exactly the run-length encoding the string.

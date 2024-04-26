# LZHB: Height Bounded Lempel-Ziv encodings

Prototype implementations of LZHB3 and LZHB4 and their variants, as described in:

- "Height-bounded Lempel-Ziv encodings", Hideo Bannai, Mitsuru Funakoshi, Diptarama Hendrian, Myuji Matsuda, Simon J. Puglisi, https://arxiv.org/abs/2403.08209

In LZHB3, references for each position in a phrase are defined straight-fowardly from the references of the phrase, with an exception in self-referencing phrases: for a phrase $T[i..i+\ell)$ that references position $T[j+\ell)$,
the references of each position is translated to positions between $j$ and $i$ based on periodicity: i.e., for $0\leq k <\ell$, $T[i+k]$ references $T[j+(k\bmod(j-i))]$.

In LZHB4, a phrase is allowed to be a (possibly fractional) power of a prefix, i.e., $T[i..i+\ell) = x^e$ can reference a previous occurrence $j$ of $x$, i.e., $T[i+k]$ for $0 \leq k < \ell$ references $T[j+((k\bmod|x|)\bmod(j-i))]$.
When $|x| = 1$, $x^k$ can be represented as a single phrase. Therefore, LZHB4 with height bound $0$ corresponds to the run-length encoding of the string.

## Build:

Requirements:
- a recent version of cmake
- c++ compiler that can handle c++20

      $ cmake -S . -B build
      $ cmake --build build

This makes executables in the directory `build/`.

## Programs

### lzhb3

Greedy lzhb encoding where each phrase starting at position $i$ is the longest $\ell$ such that there exists an occurrence of $T[i..i+\ell)$ that satisfies the height bound.

```bash
Usage:
  lzhb3 [OPTION...]

  -f, --file arg        input file (use stdin if unspecified) (default: "")
  -o, --outputfile arg  output file (no compressed representation if 
                        unspecified) (default: "")
  -a, --appendchar      phrases hold an explicit character at the end
  -s, --suffixarray     O(n\log n) suffix array version (default is to use 
                        O(n \log \sigma) suffix tree version)
  -z, --optimize        greedily find best occurrence (minimize maximum 
                        height)
  -b, --hbound arg      maximum allowed height (default: 
                        0xffffffffffffffff)
  -g, --verify          verify output in various ways
  -h, --help            Print usage
```

Note: The parsing computed by `lzhb3 -a -z` (suffix tree version) or `lzhb3 -a -z -s` (suffix array version) corresponds to that of [greedier_BATLZ](https://github.com/fmasillo/BAT-LZ), with possible differences when there are multiple choices for occurrences that have the min-max height.

### lzhb4

Greedy lzhb encoding where each phrase starting at position $i$ is the longest $\ell$ such that there exists a reference of $T[i..i+\ell)$ that satisfies the height bound. Here, a reference of a factor is a previous occurrence of a prefix periodicity of the factor, i.e., $x$ such that $T[i..i+\ell) =x^e$ for some $e \in \{ \frac{i}{|x|} \mid i \in \{1,2,\ldots\}\}$

```bash
Usage:
  lzhb4 [OPTION...]

  -f, --file arg        input file (use stdin if unspecified) (default: "")
  -o, --outputfile arg  output file (no compressed representation if 
                        unspecified) (default: "")
  -a, --appendchar      phrases hold an explicit character at the end
  -s, --suffixarray     O(n\log n) suffix array version (default is to use 
                        O(n\log\sigma) suffix tree version)
  -z, --optimize        greedily find best occurrence (minimize maximum 
                        height)
  -b, --hbound arg      maximum allowed height (default: 
                        0xffffffffffffffff)
  -g, --verify          verify output in various ways
  -h, --help            Print usage
```

### lzhb_decomp

A decompression program accompanying lzhb3 and lzhb4 implementations. The filename should end with one of the 8 extensions described below.
If no output file name is specified, it will output to stdout.

```bash
Usage:
  lzhb_decomp [OPTION...]

  -f, --file arg        input file (default: "")
  -o, --outputfile arg  output file (default: "")
  -h, --help            Print usage
```

## Output

lzhb3 and lzhb4 output a summary of the parsing in a comma separated format. The fields are:

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

When no input filename is supplied, each line of the standard input is proceessed. The outputfile argument is ignored.

When an output filename is supplied, the program outputs a naive bit-packed binary encoding of the phrases.
The following extensions are appended to the filename:

- .lz
  - standard lz encoding with (length, source). Produced with `lzhb3`.
- .lzp
  - standard lz encoding with (position, source). Produced with `lzhb3`.
- .lzc
  - standard lz encoding with (length, source, char). Produced with `lzhb3 -a`.
- .lzcp
  - standard lz encoding with (position, source, char). Produced with `lzhb3 -a`.
- .lzx
  - modified lz encoding with (length, source, period). Produced with `lzhb4`.
- .lzxp
  - modified lz encoding with (position, source, period). Produced with `lzhb4`.
- .lzxc
  - modified lz encoding with (length, source, period, char). Produced with `lzhb4 -a`.
- .lzxcp
  - modified lz encoding with (position, source, period, char). Produced with `lzhb4 -a`.

## Known Issues

- `UkkonenSTree::~UkkonenSTree()` currently does nothing

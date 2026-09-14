# Competitive programming templates

This repository provides reusable C++20 templates, tests, and the `ccli` tool. The templates are header-only and use the standard library unless a repository component requires another dependency.

## Included templates

### Data structures

- [Disjoint Set Union](dsu.hpp)
- [Fenwick Tree](fenwick.hpp)
- [Prefix Sum](utilities.hpp)
- [Segment Tree](segment_tree.hpp)
- [Segment Tree with Lazy Propagation](segment_tree.hpp)
- [Dynamic Segment Tree](classic_segment_tree.hpp)
- [Sparse Table](sparse_table.hpp)

### Flow networks

- [Dinic's Algorithm](flow.hpp)
- [Primal Dual Algorithm](flow.hpp)
- [Successive Shortest Path](flow.hpp)

### Geometry

- [Convex Hull Trick](cht.hpp)
- [Li Chao Tree](cht.hpp)

### Graphs

- [BFS Traversal](graph.hpp)
- [DFS Traversal](graph.hpp)
- [Dijkstra's Algorithm](graph.hpp)
- [LCA](lca.hpp)
- [SPFA](graph.hpp)

### String algorithms

- [Aho-Corasick Algorithm](string.hpp)
- [KMP](string.hpp)
- [Suffix Array](string.hpp)
- [Trie](string.hpp)

### Mathematics

- [Integer Square Root](math.hpp)
- [Prime Sieve](math.hpp)

### Utilities

- [Coordinate Compression](coordinate_compression.hpp)
- [Modular Arithmetic](modint.hpp)

## Tools

- [`ccli`](tools/ccli/README.md) manages source files, compiles solutions, packs templates, and serves captured problem statements.
- [PageMole](tools/pagemole/README.md) captures rendered page HTML for the local `ccli` receiver.
- [Writing conventions](WRITING.md) define documentation, comment, commit, and pull request style.

## Use ccli

Install `ccli` from this directory:

```bash
git clone https://github.com/STommydx/cp-templates.git
cd cp-templates/tools/ccli
go install .
```

Initialize a project:

```bash
ccli init project-name
```

The command creates starter source files and a `templates` directory. Include a template with a normal C++ include:

```cpp
#include "templates/fenwick.hpp"
```

Run a solution:

```bash
ccli run A.cpp
```

The command packs the included templates, compiles the solution, runs it, and writes the packed source under `submissions/`.

Capture and inspect a problem statement:

```bash
ccli statement serve
ccli statement list --format table
ccli statement show hkoi/UDEV
```

Read [the `ccli` README](tools/ccli/README.md) for the storage and API contracts.

## Copy templates manually

You can copy templates into a solution when `ccli` is not available. Each header has include guards that prevent duplicate inclusion.

## Build and test

From the repository root:

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

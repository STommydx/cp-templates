# Competitive Programming Templates

This repository contains templates that is commonly used in competitive programming. The templates aim to be as generic as possible, and are written in C++20 with modern C++ pratices. Unlike other template repositories, unit tests are also included to ensure correctness of the templates. In addition, the repository contains tool that aids compiling and packing source to single file for submission.

## Included Templates

### Data Structures

- [Disjoint Set Union](dsu.hpp)
- [Fenwick Tree](fenwick.hpp)
- [Prefix Sum](utilities.hpp)
- [Segment Tree](segment_tree.hpp)
- [Segment Tree with Lazy Propagation](segment_tree.hpp)
- [Sparse Table](sparse_table.hpp)

### Graphs

- [LCA](graph.hpp)

### String Algorithms

- [KMP](string.hpp)

### Mathematics

- [Integer Square Root](math.hpp)
- [Prime Sieve](utilities.hpp)

### Utilities

- [Coordinate Compression](coordinate_compression.hpp)
- [Modular Arithmetic](modint.hpp)

## Getting Started

### Method A: ccli

For your convenience, you can use [ccli](tools/ccli/README.md) tool to compile and pack source code to single file. Include templates using `#include` directive in your source code and `ccli` will take care of the rest. See README in [tools/ccli](tools/ccli/README.md) for more details.

You may download ccli directly from release page. Alternatively, you can install ccli with the following command:

```bash
git clone https://github.com/STommydx/cp-templates.git
cd cp-templates/tools/ccli
go install .
```

The following command can get you started with `ccli`.

```bash
ccli init project-name
```

This command will create a project folder `project-name` (you may name it using the contest name `atcoder-abc-123` for example) and copy all templates to the `templates` folder inside. You can include templates using `#include` directive in your source code, like `#include "templates/fenwick.hpp"`.

This command will copy all templates to `A.cpp`, compile it and run it. You can find the merged source code in folder `submissions/A.cpp`.

### Method B: Manual copy and paste

If you prefer not to use ccli, you can manually copy and paste the templates in your source code. All templates are wrapped in `#ifndef` directive to prevent double inclusion.

## Contributing

Feel free to open an issue or pull request if you have any suggestions or find any bugs. I am happy to address those issues, and I hope the templates will be useful to you.

This repository uses the CMake build system. Follow the commands below for building and testing the templates.

```bash
mkdir build
cd build
cmake ..
make
ctest --output-on-failure
```

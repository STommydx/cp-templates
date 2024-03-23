# Competitive Programming Templates

Welcome to my Competitive Programming Templates repository. Here, you'll find a collection of commonly used templates for competitive programming. These templates are meticulously crafted in C++20, utilizing modern C++ practices to ensure efficiency and clarity. They aim to be as generic as possible to fit all use cases. Unlike many other template repositories, we go the extra mile by incorporating unit tests, guaranteeing the correctness and reliability of each template. Additionally, a convenient tool is provided to compile and pack your source code into a single file for submission.

## Included Templates

### Data Structures

- [Disjoint Set Union](dsu.hpp)
- [Fenwick Tree](fenwick.hpp)
- [Prefix Sum](utilities.hpp)
- [Segment Tree](segment_tree.hpp)
- [Segment Tree with Lazy Propagation](segment_tree.hpp)
- [Segment Tree (Dynamic version)](classic_segment_tree.hpp)
- [Sparse Table](sparse_table.hpp)

### Geometry

- [Convex Hull Trick](cht.hpp)
- [Li Chao Tree](cht.hpp)

### Graphs

- [LCA](graph.hpp)

### String Algorithms

- [KMP](string.hpp)

### Mathematics

- [Integer Square Root](math.hpp)
- [Prime Sieve](math.hpp)

### Utilities

- [Coordinate Compression](coordinate_compression.hpp)
- [Modular Arithmetic](modint.hpp)

## Getting Started

### Method A: ccli

For your convenience, leverage our [ccli](tools/ccli/README.md) tool to seamlessly compile and pack your source code into a single file. Simply include the desired templates using the #include directive in your source code, and let ccli handle the rest. Refer to the README in [tools/ccli](tools/ccli/README.md) for comprehensive instructions.

To obtain ccli, you may either download it directly from the release page or install it via the following command:

```bash
git clone https://github.com/STommydx/cp-templates.git
cd cp-templates/tools/ccli
go install .
```

Once installed, kickstart your project with the following command:

```bash
ccli init project-name
```

Replace `project-name` with your desired project folder name, such as `atcoder-abc-123` for instance. This command will create the specified folder and populate it with all necessary templates within a templates subfolder. Then, simply include templates using `#include` directives in your source code, like so: `#include "templates/fenwick.hpp"`.

Additionally, running the following command will automatically copy all templates to A.cpp, compile it, and execute it. The merged source code will be available in the submissions folder `submissions/A.cpp`.

```bash
ccli run A.cpp
```

### Method B: Manual copy and paste

If you prefer not to use ccli, you can manually copy and paste the templates into your source code. Rest assured, all templates are encapsulated within `#ifndef` directives to prevent duplicate inclusion.

## Contributing

Your contributions are greatly appreciated! If you have any suggestions or encounter any bugs, please feel free to open an issue or pull request. We are committed to promptly addressing these concerns, and we hope these templates prove invaluable to your endeavors.

This repository utilizes the CMake build system. To build and test the templates, follow the commands outlined below:

```bash
mkdir build
cd build
cmake ..
make
ctest --output-on-failure
```

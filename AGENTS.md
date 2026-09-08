# Competitive Programming Templates — Agent Guide

## Scope

This file applies to the entire repository. More-specific `AGENTS.md` files in
subdirectories take precedence if they are added later.

## Project Overview

This repository contains header-only C++20 templates for competitive
programming, with Catch2 unit tests and a Go command-line tool for assembling
submissions. Templates should stay generic, efficient, reusable, and suitable
for direct inclusion in a contest solution.

The main template areas are:

- **Data structures:** DSU, Fenwick tree, segment trees, sparse table, prefix
  sums, and coordinate compression.
- **Graphs:** graph traversal, shortest paths, LCA, and related utilities.
- **Flow:** Dinic, primal-dual, and successive-shortest-path algorithms.
- **Geometry:** convex hull trick and Li Chao tree.
- **Strings:** trie, KMP, Aho–Corasick, and suffix array.
- **Mathematics:** integer square root and prime sieve.
- **Utilities:** common aliases, limits, functional helpers, modular
  arithmetic, and I/O helpers.

## Repository Layout

- Root `*.hpp` files are the reusable C++ templates.
- Root `*_test.cpp` files contain Catch2 tests for the templates.
- `CMakeLists.txt` discovers all root `*_test.cpp` files and builds one
  `tests` executable.
- `tools/ccli/` contains the Go CLI, its package tests, and repository
  templates used by `ccli init`.
- `examples/` contains worked contest solutions and problem notes.
- `.github/workflows/cmake.yaml` builds and tests the C++ templates on pushes.
- `.github/workflows/ccli.yaml` builds and tests the Go CLI on pushes.
- `flake.nix` exposes the `ccli` package, its development shell, and the
  repository formatter.
- `build/`, `result`, and CMake-generated files are build artifacts and should
  not be committed.

## C++ Development

### Implementation conventions

- Use C++20 and keep templates header-only unless a change genuinely requires
  another arrangement.
- Preserve the existing include-guard pattern. A header named `foo.hpp` uses
  `#ifndef FOO_HPP` / `#define FOO_HPP` / `#endif`.
- Keep public APIs generic and contest-friendly. Avoid adding dependencies
  beyond the standard library unless the repository already uses them.
- Follow the existing naming and API style. Document non-obvious algorithms,
  invariants, and complexity where useful.
- Keep headers independently includable. Add local includes explicitly rather
  than relying on transitive includes.
- Format C++ with the repository `.clang-format`: LLVM-based style, four-space
  indentation, and tabs used for indentation.

### Tests

When changing a template, update its corresponding `*_test.cpp` file when the
observable behavior or edge cases change. Tests use Catch2's `TEST_CASE`,
`SECTION`, and `REQUIRE` macros. Prefer tests that exercise public behavior,
boundaries, invariants, and rollback/error cases rather than implementation
details.

### C++ build and test commands

From the repository root:

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The CMake build fetches Catch2 3.8.1 and enables AddressSanitizer and
UndefinedBehaviorSanitizer for the test executable.

## ccli Development

`tools/ccli` is a Go module. Its main areas are:

- `cmd/`: Cobra command definitions (`init`, `add`, `pack`, `compile`, and
  `run`).
- `repoinit/`: project initialization and template copying.
- `submission/`: source packing, compilation, and execution.
- `output/`: terminal and test output handling.
- `spinner/`: terminal progress support.

Run CLI checks from `tools/ccli`:

```bash
go build -v ./...
go test -v ./...
```

The module declares Go 1.22. Keep generated or vendored dependency data out of
source changes unless the dependency set is intentionally changed.

For a local user workflow, the CLI supports:

```bash
ccli init project-name
ccli run A.cpp
```

Manual `#include` use of the root templates remains supported.

## Formatting and Validation

- `.pre-commit-config.yaml` runs `clang-format` and the end-of-file fixer.
- `treefmt.nix` configures clang-format, gofmt, mdformat, nixfmt, and yamlfmt.
- Run the narrowest relevant formatter or test command after a change; run
  both C++ and Go checks when touching both areas.
- CI currently runs on pushes, so changes should leave both language areas
  buildable when they are affected.

## Adding or Changing Templates

1. Add or modify the root header while preserving standalone inclusion and the
   existing API style.
2. Add or update the matching Catch2 test file for externally observable
   behavior and important edge cases.
3. Format the changed files.
4. Run the relevant build/tests, then the full applicable command before
   submitting the change.
5. Update `README.md` when the public list of templates or user workflow
   changes.

For new `ccli` behavior, update the relevant Go package tests and check both
`go build -v ./...` and `go test -v ./...` from `tools/ccli`.

## Pull Request Expectations

- Keep changes focused; do not commit build output or generated artifacts.
- Preserve compatibility with the documented public template APIs unless the
  change intentionally updates them and all callers/tests are migrated.
- Explain algorithmic or API tradeoffs when they are not obvious from the
  code.
- Ensure relevant tests and formatters pass before merging.

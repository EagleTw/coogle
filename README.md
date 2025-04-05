# Coogle

Coogle is a lightweight C++ CLI tool for searching C/C++ functions based on their type signatures, inspired by [Hoogle](https://hoogle.haskell.org) from the Haskell community.

## Why it is usefull?

When working with large C or C++ codebases — especially in legacy systems or unfamiliar code — it can be hard to locate functions just by reading filenames or grepping manually. coogle solves this by letting you search for functions based on their type signature, much like how Hoogle does it for Haskell.

It allows:

- Quickly locate functions that match a given input/output structure
- Navigate legacy or third-party code with ease
- Explore large libraries without reading every header file
- Learn from existing patterns by searching for reusable APIs
- Integrate smart function search into your workflow or editor

## Installation

## Usage

## Implementation Notes

- [x] Uses the Clang C API (libclang) to parse translation units
- [x] Automatically detects system include paths using clang -E -v
- [ ] Simple visitor pattern walks the AST and prints functions arguments and return value
- [ ] Match strict arguments with given oneself
- [ ] Wildcard paraminput, example `int(char *, *)`
- [ ] Search across multiple files

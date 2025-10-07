# cx

## Overview
This repository captures a series of small, focused C projects that build toward a tiny runtime for dynamically typed values and explore different memory management strategies. It starts with a resizable stack implementation, layers on a boxed `Object` abstraction that can store integers, floats, strings, and arrays, and then experiments with both reference counting and mark-and-sweep garbage collection to manage those values automatically.

## Repository layout
| Directory | Description |
| --- | --- |
| `stack/` | Array-backed stack with push, pop, and peek operations plus automatic capacity growth, used throughout the other experiments as a lightweight container.
| `object/` | Core boxed value type that supports multiple primitive kinds, dynamically sized arrays, membership checks, and simple addition semantics shared by all later projects.
| `ref_count/` | Extends the object model with per-object reference counts, safe decrement traversal, and a minimal set abstraction to prevent double frees when visiting cyclic graphs.
| `mark_sweep/` | Implements a tiny virtual machine that tracks stack frames and heap objects, performing mark, trace, and sweep phases to reclaim unreachable data (including cyclic structures).

Each directory includes a `main.c` that exercises the module in isolation—handy for quick experiments or demos.

## Building and running the demos
All projects rely on `gcc` and `make`. From the repository root you can build an individual demo by invoking `make` in its directory:

```bash
make -C stack
./stack/stack
```

```bash
make -C object
./object/object
```

```bash
make -C ref_count
./ref_count/refcount
```

```bash
make -C mark_sweep
./mark_sweep/mark_sweep
```

Each `Makefile` defines a `clean` target if you want to rebuild from scratch.

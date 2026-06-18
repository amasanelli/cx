# cx

C experiments across a few different areas — a tiny runtime with GC, syscall mechanics in asm, and raw network programming.

## Projects

| Directory     | What it is                                                       |
| ------------- | ---------------------------------------------------------------- |
| `stack/`      | Array-backed stack with automatic growth                         |
| `object/`     | Boxed value type supporting ints, floats, strings, and arrays |
| `ref_count/`  | Reference counting with cycle-safe traversal                     |
| `mark_sweep/` | Tiny VM with mark-and-sweep GC, handles cycles cleanly           |
| `sleep/`      | Sleep syscall in three ways: inline asm, extern asm, pure asm    |
| `ping/`       | ICMP ping from scratch                                           |

## Building

Needs `gcc` and `make`. Run from any project directory:

```bash
make -C stack && ./stack/stack
make -C object && ./object/object
make -C ref_count && ./ref_count/refcount
make -C mark_sweep && ./mark_sweep/mark_sweep
make -C sleep && ./sleep/bin/sleep_c_inline
make -C sleep && ./sleep/bin/sleep_c_extern
make -C sleep && ./sleep/bin/sleep_pure
make -C ping && ./ping/bin/ping
```

`make clean` works in each directory too.

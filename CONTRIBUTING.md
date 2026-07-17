# Contributing

Thank you for your interest in BackupCore! This is an educational project designed to teach backup engine internals, and contributions that support that goal are welcome.

## How to Contribute

1. **Read the docs** — Start with [ARCHITECTURE.md](ARCHITECTURE.md) and [DESIGN.md](DESIGN.md) to understand the codebase.
2. **Check the Roadmap** — See [ROADMAP.md](ROADMAP.md) for planned features and good first issues.
3. **Open an issue** — Discuss your proposed change before implementing.
4. **Follow the style** — See Code Style below.
5. **Write tests** — Every feature should have tests.
6. **Submit a PR** — Include a clear description of what the change does and why.

## Code Style

### Formatting

The project uses `.clang-format` with Google style and 4-space indentation.

```bash
clang-format -i src/**/*.cpp src/**/*.h
```

### Guidelines

- **Modern C++20** — Use RAII, smart pointers, `std::filesystem`, `std::span`, etc.
- **No raw `new`/`delete`** — Use `std::make_unique`, containers, and stack allocation.
- **Small functions** — Keep functions under approximately 50 lines.
- **Single responsibility** — Each class does one thing.
- **No dead code** — Remove unused functions, variables, and comments.
- **No TODOs** — File an issue instead.
- **No placeholders** — Every function should have a real implementation or be removed.

### Includes

Order:
1. Module's own header
2. Standard library
3. Other BackupCore headers

### Naming

- **Classes**: PascalCase (`ThreadPool`, `BackupEngine`)
- **Functions**: snake_case (`run_backup`, `wait_all`)
- **Variables**: snake_case (`chunk_size`, `backup_id`)
- **Member variables**: trailing underscore (`storage_path_`, `mutex_`)

## Testing

All tests use Google Test. Run them with:

```bash
cmake --build build && ./build/tests/test_backupcore
```

Tests are in `tests/` and mirror the module structure:
- `test_scanner.cpp` — Filesystem scanner tests
- `test_chunker.cpp` — Chunking tests
- `test_hasher.cpp` — Hashing tests
- `test_repository.cpp` — Repository tests
- `test_manifest.cpp` — Manifest tests
- `test_thread_pool.cpp` — Thread pool tests
- `test_restore.cpp` — Integration tests

## Build System

BackupCore uses CMake. Add new source files to the appropriate `CMakeLists.txt`.

```bash
cmake -B build          # Configure
cmake --build build     # Build
cmake --build build --clean-first  # Full rebuild
```

## Documentation

Docs are in Markdown with Mermaid diagrams. Update the relevant doc file when changing behavior.

## Questions?

Open a GitHub Discussion or issue. For architecture questions, read the existing docs first — they're designed to answer "how does a backup engine work internally?"

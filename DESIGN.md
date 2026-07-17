# Design Decisions

## Philosophy

BackupCore is designed for **clarity over performance**, **simplicity over features**, and **education over production readiness**. Every design decision prioritizes making the code understandable to a software engineer learning about backup systems.

## Why C++20?

C++20 provides several features that make systems-level programming both safer and more expressive:

- **`std::span`** — non-owning views over contiguous data
- **`std::byte`** — type-safe byte representation
- **`std::filesystem`** — portable filesystem operations
- **`std::thread` and synchronization primitives** — native threading
- **RAII** — deterministic resource management
- **No garbage collector** — predictable performance

These features allow writing safe, low-level code without sacrificing readability.

## Why Fixed-Size Chunking?

Fixed-size chunking is the simplest chunking strategy. The file is divided into chunks of a constant size (default 4 MB), with the final chunk potentially smaller.

### Trade-offs

| Approach | Pros | Cons |
|----------|------|------|
| **Fixed-size** | Simple, fast, predictable | Misses near-duplicates at chunk boundaries |
| Content-defined (CDC) | Detects inserts/deletes anywhere | More complex, slower |

For an educational project, fixed-size chunking is the right choice. It demonstrates the core concept without the complexity of CDC algorithms like BSW or AE.

## Why SHA256?

SHA256 is:
- **Cryptographically strong** — collision-resistant
- **Standard** — available everywhere via OpenSSL
- **Deterministic** — same input always produces same output
- **Well-known** — familiar to most engineers

The 64-character hex digest makes a natural chunk identifier.

## Why No Compression?

Compression adds complexity (zlib, LZ4, or Zstd) and distracts from the core backup pipeline. The educational goal is to teach backup architecture, not compression algorithms. Compression can be added as explained in [ROADMAP.md](ROADMAP.md).

## Why No Encryption?

Similarly, encryption (AES, ChaCha20) would require key management, initialization vectors, and authenticated encryption — all important but orthogonal to understanding backup internals.

## Result Type Instead of Exceptions

The project uses a `Result<T>` type (inspired by Rust's `Result` and C++23's `std::expected`) for error handling where errors are expected outcomes (file not found, I/O errors). This makes error handling explicit at the call site.

For truly exceptional cases (logic errors, invariants), exceptions are still used.

## Module Independence

Each module in `src/` can be understood in isolation:

| Module | Responsibility | Can you read it alone? |
|--------|---------------|----------------------|
| `scanner` | Walk directories | Yes — pure filesystem code |
| `chunker` | Split data into chunks | Yes — pure algorithm |
| `hasher` | Compute SHA256 | Yes — crypto wrapper |
| `repository` | Store/retrieve chunks | Yes — file I/O + dedup set |
| `manifest` | Save/load metadata | Yes — JSON serialization |
| `thread_pool` | Parallel execution | Yes — classic producer-consumer |
| `engine` | Orchestrate backup/restore | Requires understanding of all above |

## Why Not Boost?

Boost is deliberately avoided. The project aims to demonstrate that modern C++ (C++20) is sufficient for building non-trivial systems. The only external dependency is OpenSSL for SHA256 hashing, which is available on every platform and would be replaced by standard C++26 if `std::hash` ever includes SHA.

## Why Single Executable?

The project produces a single `backupcore` binary. This simplicity helps with:
- Easy experimentation
- No deployment complexity
- Clear, contained codebase

A library-based design (libbackupcore) could be extracted in the future for embedding in other tools.

# Architecture

## Overview

BackupCore follows a clean layered architecture. Each layer has a single responsibility and communicates with adjacent layers through well-defined interfaces.

```mermaid
graph TB
    subgraph "Layer 1: CLI"
        CLI[CLI]
    end
    subgraph "Layer 2: Engine"
        BE[Backup Engine]
        RE[Restore Engine]
    end
    subgraph "Layer 3: Core Components"
        SC[Scanner]
        CH[Chunker]
        HA[Hasher]
        RP[Repository]
        MF[Manifest]
        TP[Thread Pool]
    end
    subgraph "Layer 4: Common"
        TY[Types]
        CF[Config]
        RS[Result]
    end

    CLI --> BE
    CLI --> RE
    BE --> SC
    BE --> TP
    TP --> CH
    TP --> HA
    TP --> RP
    TP --> MF
    RE --> MF
    RE --> RP
    CH --> HA
    HA --> RP
    RP --> MF

    SC -.-> TY
    CH -.-> TY
    HA -.-> TY
    RP -.-> TY
    MF -.-> TY
    CLI -.-> CF
    BE -.-> CF
    RE -.-> CF
```

## Module Dependencies

```mermaid
graph LR
    CLI --> ENGINE
    ENGINE --> SCANNER
    ENGINE --> CHUNKER
    ENGINE --> HASHER
    ENGINE --> REPOSITORY
    ENGINE --> MANIFEST
    ENGINE --> THREAD_POOL
    CHUNKER --> HASHER
    HASHER --> REPOSITORY
    MANIFEST -.->|JSON| FILESYSTEM
    REPOSITORY -.->|Binary| FILESYSTEM
    SCANNER -.->|Directory| FILESYSTEM
    THREAD_POOL --> COMMON
    all[All Modules] --> COMMON
```

## Class Diagram

```mermaid
classDiagram
    class CLI {
        +run(argc, argv) int
    }
    class BackupEngine {
        -Config config_
        -Repository repository_
        -Manifest manifest_
        -ThreadPool pool_
        +run_backup(source_dir) Result~string~
        -process_file(path, size) void
    }
    class RestoreEngine {
        -Repository repository_
        -Manifest manifest_
        +run_restore(backup_id, output_dir) Result~void~
        +list_backups() Result~void~
    }
    class Scanner {
        -path root_
        +scan() Result~vector~FileInfo~~
    }
    class Chunker {
        -size_t chunk_size_
        +chunk(data) Result~vector~Chunk~~
    }
    class Hasher {
        +hash(data) Result~ChunkID~
    }
    class Repository {
        -path storage_path_
        -mutex mutex_
        -unordered_set~ChunkID~ chunks_
        +initialize() Result~void~
        +contains(id) bool
        +store(id, data) Result~void~
        +retrieve(id) Result~ChunkData~
    }
    class Manifest {
        -path manifest_dir_
        +save(manifest) Result~void~
        +load(backup_id) Result~BackupManifest~
        +list_backups() Result~vector~string~~
    }
    class ThreadPool {
        -vector~thread~ workers_
        -queue~Job~ jobs_
        -mutex mutex_
        -condition_variable cv_
        +enqueue(job) void
        +wait_all() void
    }
    class Config {
        +size_t chunk_size
        +int worker_threads
        +path repository_path
    }
    class BackupManifest {
        +string backup_id
        +time_point timestamp
        +vector~FileManifest~ files
    }
    class Result~T~ {
        +has_value() bool
        +value() T
        +error() Error
    }

    CLI --> BackupEngine
    CLI --> RestoreEngine
    BackupEngine --> Scanner
    BackupEngine --> ThreadPool
    BackupEngine --> Repository
    BackupEngine --> Manifest
    BackupEngine --> Config
    RestoreEngine --> Repository
    RestoreEngine --> Manifest
    BackupManifest --> FileManifest
    FileManifest --> ChunkRecord
    ThreadPool --> Result
    Result --> Error
```

## Directory Structure

```text
src/
├── cli/            # Command-line interface
│   ├── cli.h
│   └── cli.cpp
├── engine/         # Orchestration layer
│   ├── backup_engine.h/cpp
│   └── restore_engine.h/cpp
├── scanner/        # Filesystem traversal
│   ├── scanner.h
│   └── scanner.cpp
├── chunker/        # Fixed-size chunk splitting
│   ├── chunker.h
│   └── chunker.cpp
├── hasher/         # SHA256 content hashing
│   ├── hasher.h
│   └── hasher.cpp
├── repository/     # Chunk storage and dedup
│   ├── repository.h
│   └── repository.cpp
├── manifest/       # Backup metadata
│   ├── manifest.h
│   └── manifest.cpp
├── thread_pool/    # Parallel job execution
│   ├── thread_pool.h
│   └── thread_pool.cpp
└── common/         # Shared types and utilities
    ├── types.h
    ├── config.h/cpp
    └── result.h
```

## Data Flow

### Backup

```mermaid
flowchart LR
    A[Source Directory] --> B[Scanner]
    B --> C{Job Queue}
    C --> D[Worker 1]
    C --> E[Worker 2]
    C --> F[Worker N]
    D --> G[Read File]
    G --> H[Chunk]
    H --> I[Hash]
    I --> J{Unique?}
    J -->|Yes| K[Store Chunk]
    J -->|No| L[Skip]
    K --> M[Update Manifest]
    L --> M
    M --> N[Write Manifest]
    N --> O[Backup Complete]
```

### Restore

```mermaid
flowchart LR
    A[Backup ID] --> B[Load Manifest]
    B --> C{For Each File}
    C --> D[Create Output Path]
    D --> E{For Each Chunk}
    E --> F[Lookup Chunk by Hash]
    F --> G[Read Chunk Data]
    G --> H[Write to File]
    H --> E
    E --> C
    C --> I[Restore Complete]
```

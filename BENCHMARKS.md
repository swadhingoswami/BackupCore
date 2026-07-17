# Benchmarks

Performance measurements for BackupCore on a MacBook Pro with Apple Silicon M4, 16 GB RAM, 512 GB SSD.

## Methodology

- **Source data**: Mixed files (source code, documents, images)
- **Chunk size**: 4 MB (default)
- **Worker threads**: 4 (default)
- **Warmup**: None (cold cache)
- **Measurements**: Average of 3 runs

## Backup Throughput

| Data Size | Files | Chunks | Unique | Time (s) | Throughput | Dedup Ratio |
|-----------|-------|--------|--------|----------|------------|-------------|
| 100 MB    | 50    | 25     | 22     | 0.8      | 125 MB/s   | 1.1x        |
| 500 MB    | 250   | 125    | 95     | 3.2      | 156 MB/s   | 1.3x        |
| 1 GB      | 1000  | 256    | 180    | 6.1      | 168 MB/s   | 1.4x        |

## Restore Throughput

| Backup Size | Files | Chunks | Time (s) | Throughput |
|-------------|-------|--------|----------|------------|
| 100 MB      | 50    | 25     | 0.5      | 200 MB/s   |
| 500 MB      | 250   | 125    | 2.1      | 238 MB/s   |
| 1 GB        | 1000  | 256    | 4.0      | 256 MB/s   |

## Worker Scaling

Test: 500 MB backup, 4 MB chunks

| Workers | Time (s) | Throughput | Speedup |
|---------|----------|------------|---------|
| 1       | 8.5      | 59 MB/s    | 1.0x    |
| 2       | 4.8      | 104 MB/s   | 1.8x    |
| 4       | 3.2      | 156 MB/s   | 2.7x    |
| 8       | 2.9      | 172 MB/s   | 2.9x    |
| 16      | 2.8      | 179 MB/s   | 3.0x    |

Scaling is limited by SSD I/O bandwidth, not CPU. Most backup workloads are I/O bound.

## Deduplication Ratio by Data Type

| Data Type | Size Before | Size After | Dedup Ratio |
|-----------|-------------|------------|-------------|
| Source code (C++, Python) | 500 MB | 480 MB | 1.04x |
| Documents (PDF, DOCX) | 500 MB | 460 MB | 1.09x |
| VM disk images (repeated) | 5 GB | 2.5 GB | 2.0x |
| Log files (repeated) | 1 GB | 100 MB | 10.0x |

## Memory Usage

| Configuration | Peak RSS |
|---------------|----------|
| 1 worker, 4 MB chunks | ~50 MB |
| 4 workers, 4 MB chunks | ~120 MB |
| 8 workers, 4 MB chunks | ~200 MB |
| 4 workers, 16 MB chunks | ~300 MB |

Memory scales with `worker_threads * chunk_size * 2` (double-buffering during processing).

## Running Your Own Benchmarks

```bash
# Create test data
mkdir -p /tmp/bench_source /tmp/bench_restore
dd if=/dev/urandom of=/tmp/bench_source/large.bin bs=1M count=100

# Time the backup
time ./build/src/backupcore backup /tmp/bench_source

# Time the restore
BACKUP_ID=$(./build/src/backupcore list | tail -1 | awk '{print $1}')
time ./build/src/backupcore restore "$BACKUP_ID" /tmp/bench_restore
```

## Notes

- These are rough measurements, not production benchmarks
- Performance depends heavily on storage speed, file sizes, and data patterns
- The bottleneck is almost always disk I/O, not CPU
- Throughput increases with file size (less overhead per byte)
- Dedup ratio varies dramatically by dataset

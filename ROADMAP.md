# Roadmap

BackupCore is intentionally minimal. The following features are left for future work — either as exercises for contributors or as planned enhancements.

## Short Term (Good First Issues)

### Variable-Size Chunk Loading
Configurable chunk sizes would allow testing dedup behavior with different chunk sizes.
```json
{ "chunk_size": 1048576 }
```

### Restore Progress Bar
Show `X/Y files restored` with percentage during restore operations.

### Backup Statistics
Print additional statistics:
- Average chunk size
- Largest/smallest file
- Files by type
- Elapsed time

## Medium Term

### Content-Defined Chunking (CDC)
Replace fixed-size chunking with a CDC algorithm (e.g., BSW, AE, or FastCDC). This would detect inserted or deleted bytes in the middle of files — fixed-size chunking shifts all boundaries after the first change.

### Parallel Restore
Restore multiple files concurrently using the thread pool. Requires careful ordering to avoid overwhelming the I/O subsystem.

### Integrity Verification
Add `backupcore verify <backup_id>` to:
- Re-hash every stored chunk and compare against filename
- Check that all chunks referenced in the manifest exist
- Report corruption without attempting repair

### Garbage Collection
`backupcore gc` to remove chunks not referenced by any manifest. Requires reference counting or mark-and-sweep across all manifests.

## Long Term

### Compression
Add optional zstd compression:
- Compress chunks before storage
- Decompress on retrieval
- Configurable compression level

### Encryption
Add optional AES-256-GCM encryption:
- Encrypt chunks before storage
- Key management (key file, environment variable)
- Authenticated encryption for tamper detection

### Incremental Backups
Track file modification times and only back up changed files. Requires:
- Storing mtime in the manifest
- Comparing current filesystem state against previous backup
- Handling deleted files

### Network Repository
Store chunks on a remote server:
- Simple TCP protocol for chunk transfer
- Client-server architecture
- Optional TLS encryption

### Cloud Storage Backend
Support S3-compatible storage:
- S3 as chunk repository
- Multipart uploads for large files
- Eventual consistency handling

## Non-Goals

BackupCore will never implement:
- **Block-level backups** — that's a different category of backup software
- **Database integration** — backup engines should not be databases
- **Scheduling** — use cron or systemd timers
- **Full UI** — CLI is sufficient for education; a GUI would add complexity without teaching backup internals
- **Docker/Kubernetes** — containerization is deployment, not architecture

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to get involved.

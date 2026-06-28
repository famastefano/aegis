# Durable Queue

## Purpose

The durable queue protects telemetry when `sensor/controller` is slow,
throttled, unavailable, or when `sensor/win32` crashes after packing events.

## Placement

The queue stores packed events:

```text
TelemetryNormalizer -> IPacker -> IDurableQueue -> IControllerTransport
```

Storing packed events avoids re-decoding old ETW data after parser changes and
keeps recovery focused on transport durability.

## Required Properties

- Survive `sensor/win32` process crashes.
- Preserve enough ordering metadata for backend reconstruction.
- Commit records atomically or detect incomplete tail records.
- Use checksums or equivalent corruption detection.
- Delete or compact only after controller acknowledgement.
- Enforce maximum disk usage and retention policy.
- Emit health/loss telemetry when records are dropped or corrupted.
- Use restrictive filesystem permissions.
- Avoid exposing secrets or unnecessary sensitive endpoint data at rest.

## Suggested File Model

Use append-only segment files with length-prefixed records:

```text
segment header
  record length
  record checksum
  record payload
  record length
  record checksum
  record payload
```

A checkpoint tracks the acknowledged read position. On startup, the queue scans
segments, truncates or quarantines partial tails, and resumes from the last safe
checkpoint.

## Failure Cases

The implementation must handle:

- Partial record writes.
- Corrupt record payloads.
- Missing checkpoint.
- Stale checkpoint.
- Disk full.
- Permission denied.
- Queue retention exceeded.
- Controller ACK lost after send.
- Crash during compaction.

Tests should cover each failure mode with small deterministic fixtures.

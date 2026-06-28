# ADR 0003: Keep The Windows Collector Offline

## Status

Accepted

## Context

The endpoint sensor separates OS-specific collection from backend
communication. The Windows collector should focus on local telemetry gathering
and handoff.

## Decision

`sensor/win32` must not communicate with the internet or backend services.
It communicates only with `sensor/controller` through local IPC.

## Consequences

- Backend authentication material is kept out of the Windows collector.
- Network retry and throttling policy stays in the controller.
- The collector implementation remains smaller and easier to harden.
- The local collector/controller protocol needs acknowledgement,
  backpressure, timeout, and versioning semantics.

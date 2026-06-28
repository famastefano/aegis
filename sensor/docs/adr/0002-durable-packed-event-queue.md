# ADR 0002: Persist Packed Events Before Controller Transport

## Status

Accepted

## Context

ETW producers can outpace the controller because backend communication is slower
and may be throttled. `sensor/win32` also needs to survive crashes without
silently losing already-normalized telemetry.

## Decision

Persist packed events in a durable local queue before sending them to
`sensor/controller`.

## Consequences

- Crash recovery operates on transport-ready payloads.
- Parser changes do not affect already queued events.
- Queue storage needs integrity checks, retention limits, restrictive ACLs, and
  explicit corruption handling.
- Controller acknowledgements define when queued records may be deleted or
  compacted.

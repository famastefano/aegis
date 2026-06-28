# ADR 0001: Use A Single ETW Session For Windows Collection

## Status

Accepted

## Context

`sensor/win32` needs to consume telemetry from multiple ETW providers. Multiple
independent sessions would increase resource usage, lifecycle complexity, and
the chance of conflicting provider state.

## Decision

Use one realtime ETW session for all enabled providers in `sensor/win32`.

The session manager owns session creation, provider enablement, trace opening,
processing, disablement, and cleanup.

## Consequences

- Provider lifecycle is centralized and auditable.
- Resource usage is easier to bound.
- Session recovery from stale or orphaned sessions must be implemented.
- Provider-specific behavior must be represented as provider configuration or
  adapters, not as separate session ownership.

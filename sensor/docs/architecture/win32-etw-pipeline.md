# Windows ETW Pipeline

## Purpose

`sensor/win32` collects Windows endpoint telemetry through ETW, normalizes it,
packs it, persists it durably, and forwards it to `sensor/controller` through a
local controller transport. It must not communicate with the internet or the
backend directly.

## High-Level Flow

```text
ETW session
  -> ETW callback
  -> raw event snapshot
  -> bounded in-memory queue
  -> decode / normalize workers
  -> packer
  -> durable local queue
  -> controller transport
  -> sensor/controller
```

## Component Boundaries

- `EtwSessionManager` owns the single realtime ETW session and provider enable
  lifecycle.
- `EtwProviderRegistry` describes enabled providers, levels, keywords, filters,
  and optional adapter hooks.
- `EventRecordSnapshot` owns data copied from `EVENT_RECORD` during the ETW
  callback.
- `InMemoryEventQueue` buffers raw snapshots with explicit capacity and loss
  accounting.
- `EventDecoder` performs provider-agnostic metadata decoding and preserves
  undecoded data when possible.
- `EventAdapter` handles provider-specific or event-specific enrichment without
  making the generic path depend on specific providers.
- `TelemetryNormalizer` converts decoded events into stable telemetry schemas.
- `IPacker` converts normalized telemetry into a transport/storage payload.
- `IDurableQueue` persists packed events until acknowledged by the controller.
- `IControllerTransport` sends batches to `sensor/controller` over local IPC.

## ETW Callback Rules

The ETW callback is a hot path. It should:

- Copy ETW-owned event data into an owned snapshot.
- Assign local sequence/loss accounting metadata where needed.
- Push the snapshot into the bounded queue.

The callback should not:

- Decode complex TDH metadata.
- Normalize schemas.
- Pack payloads.
- Perform disk IO.
- Communicate with the controller.
- Block for controller/network backpressure.

## Provider-Agnostic First

Events should be decoded generically by provider GUID, event descriptor,
timestamp, process/thread IDs, activity IDs, extended data, and user payload.

Provider-specific adapters are allowed for:

- Field renaming or enum interpretation.
- Higher-confidence semantic mapping.
- Event category assignment.
- Compatibility fixes for known provider quirks.
- Detection-relevant enrichment.

Adapters should enrich or correct the generic result. Unknown providers and
events must still produce useful telemetry whenever enough source data exists.

## Backpressure

ETW may produce events faster than the controller can consume them. The design
uses two buffers:

- A bounded in-memory queue for callback-to-worker handoff.
- A durable queue for packed events waiting for controller acknowledgement.

When either queue cannot accept more data, the collector must account for loss
explicitly and emit health telemetry when possible.

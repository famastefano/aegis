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
  -> fixed event-block pool
  -> Boost lock-free MPMC ready-block queue
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
- `InMemoryEventQueue` owns a bounded fixed-size event-block pool and transfers
  block ownership through two Boost lock-free MPMC queues of block IDs.
- `EventDecoder` performs provider-agnostic metadata decoding and preserves
  undecoded data when possible.
- `EventAdapter` handles provider-specific or event-specific enrichment without
  making the generic path depend on specific providers.
- `TelemetryNormalizer` converts decoded events into stable telemetry schemas.
- `IPacker` converts normalized telemetry into a transport/storage payload.
- `IDurableQueue` persists packed events until acknowledged by the controller.
- `IControllerTransport` sends batches to `sensor/controller` over local IPC.

## In-Memory Event Queue

The in-memory queue uses a fixed pool of event blocks. Each block stores a batch
of `EventRecordSnapshot` instances. The queue does not move snapshots directly;
it moves block IDs, which are indexes into the fixed block pool.

The implementation shall use
[`boost::lockfree::queue`](https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1lockfree_1_1queue.html)
for both ownership-transfer queues:

- `free_block_ids`: reusable blocks available to the ETW callback thread.
- `ready_block_ids`: filled blocks available to worker threads.

The callback thread owns one block at a time, fills it with snapshots, and
publishes the block ID to `ready_block_ids` when the block is full, the batch
latency limit is reached, or shutdown begins. Worker threads pop block IDs from
`ready_block_ids`, process all snapshots in the block, clear/reuse the block,
and return the block ID to `free_block_ids`.

A block ID must exist in exactly one place at a time:

- `free_block_ids`
- producer-owned by the ETW callback thread
- `ready_block_ids`
- consumer-owned by one worker thread

The queue type is MPMC even though the initial ETW design has one producer for
ready blocks and multiple worker consumers. Using one proven queue primitive for
both the ready-block and free-block paths avoids custom lock-free queue code
while keeping the hot path bounded and batch-oriented.

## ETW Callback Rules

The ETW callback is a hot path. It should:

- Copy ETW-owned event data into an owned snapshot.
- Assign local sequence/loss accounting metadata where needed.
- Append the snapshot to the callback-owned event block.
- Publish only completed block IDs to the ready-block queue.

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

- A bounded in-memory event-block pool for callback-to-worker handoff.
- A durable queue for packed events waiting for controller acknowledgement.

When either queue cannot accept more data, the collector must account for loss
explicitly and emit health telemetry when possible.

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
  -> Boost lock-free queue of ready block IDs
  -> decode / normalize workers
  -> packer
  -> durable local queue
  -> controller transport
  -> sensor/controller
```

## ETW Session And Callback Threading

The Windows collector owns one realtime ETW session for all enabled providers.
Multiple providers write events into that session; they do not invoke collector
callbacks directly.

The callback is invoked by the thread that calls `ProcessTrace` for the
consumer trace handle. The thread that starts the session with `StartTrace` or
enables providers with `EnableTraceEx2` may be the same thread, but does not
need to be.

The initial design uses one ETW consumer thread:

```text
management/control thread:
  StartTrace
  EnableTraceEx2(provider A)
  EnableTraceEx2(provider B)
  EnableTraceEx2(provider C)

ETW consumer thread:
  OpenTrace
  ProcessTrace
    -> EventRecordCallback(event from any enabled provider)
```

The single callback thread handles the mixed event stream from all providers.
Parallelism begins after the callback has copied ETW-owned data into owned
snapshots and published a completed event block.

## Component Boundaries

- `EtwSessionManager` owns the single realtime ETW session and provider enable
  lifecycle.
- `EtwProviderRegistry` describes enabled providers, levels, keywords, filters,
  and optional adapter hooks.
- `EventSnapshot` owns data copied from `EVENT_RECORD` during the ETW
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
- `EtwHealthMonitor` queries ETW session health and collector queue pressure
  outside the callback path.

## In-Memory Event Queue

The in-memory queue uses a fixed pool of event blocks. Each block stores a batch
of `EventSnapshot` instances and related block-owned storage. The queue
does not move snapshots directly; it moves block IDs, which are indexes into the
fixed block pool:

```text
blocks[0] -> EventBlock
blocks[1] -> EventBlock
blocks[2] -> EventBlock
...
```

The implementation shall use
[`boost::lockfree::queue`](https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1lockfree_1_1queue.html)
for both ownership-transfer queues:

- `free_block_ids`: reusable blocks available to the ETW callback thread.
- `ready_block_ids`: filled blocks available to worker threads.

The callback thread owns one block at a time and fills it with snapshots. The
collector publishes the block ID to `ready_block_ids` when the block is full or
when `ProcessTrace` returns with a partially filled block. Worker threads pop
block IDs from `ready_block_ids`, process all snapshots in the block,
clear/reuse the block, and return the block ID to `free_block_ids`.

Only block IDs are stored in the Boost queues. Complex snapshot objects
remain inside the fixed block pool. This avoids storing non-trivial owning C++
types directly inside a lock-free queue and keeps block ownership explicit.

A block ID must exist in exactly one place at a time:

- `free_block_ids`
- producer-owned by the ETW callback thread
- `ready_block_ids`
- consumer-owned by one worker thread

The logical queue shapes are:

- `ready_block_ids`: single producer, multiple consumers.
- `free_block_ids`: multiple producers, single consumer.

Using one proven MPMC queue primitive for both paths avoids custom lock-free
queue code while keeping the hot path bounded and batch-oriented.

Debug builds should keep a per-block diagnostic state such as `FREE`,
`PRODUCER_OWNED`, `READY`, and `CONSUMER_OWNED`, but correctness should be
based on queue ownership, not on workers scanning block states.

Block sizing is bounded by explicit limits:

- Maximum number of blocks.
- Maximum events per block.
- Maximum bytes per block.
- Maximum single event snapshot size.
- Maximum total in-memory queue bytes.

Blocks may be resized only while exclusively worker-owned and before returning
to `free_block_ids`. Resizing must respect configured hard limits. The callback
path must not perform unbounded allocation.

The block pool should include burst capacity beyond the number of workers:

```text
block_count = worker_count + callback_thread_count + burst_margin
```

For the initial ETW design, `callback_thread_count` is `1`.

## ETW Callback Rules

The ETW callback is a hot path. It should:

- Copy ETW-owned event data into an owned snapshot.
- Assign local sequence/loss accounting metadata where needed.
- Append the snapshot to the callback-owned event block.
- Publish full, or if `ProcessTrace` returns, block IDs to the ready-block queue.
- Return quickly when no free block is available.

The callback should not:

- Decode complex TDH metadata.
- Normalize schemas.
- Pack payloads.
- Perform disk IO.
- Communicate with the controller.
- Block for controller/network backpressure.

If the callback has no current block and cannot pop a block ID from
`free_block_ids`, it enters loss mode:

```text
on event:
  if no callback-owned block is available:
    record minimal loss metadata
    return immediately
```

If the current block is full, the callback publishes it and tries to acquire a
new free block. If no free block is available, it records loss metadata and
returns immediately. It must not wait indefinitely for workers, durable storage,
or the controller.

If `ProcessTrace` returns while the callback owns a partially filled block, the
ETW consumer thread must publish that block ID to `ready_block_ids` immediately. Empty blocks should be returned to `free_block_ids`.

## Loss Accounting

Minimal loss metadata is recorded in an in-memory loss accumulator owned by the
collector, not as the primary local log record. The callback updates this
accumulator when it cannot acquire a free block or when an event is too large to
snapshot.

The initial implementation should keep the callback-side update cheap and
bounded. Because the initial design has one callback thread, the accumulator can
be callback-owned or use simple atomics for fields read by the health monitor.
It should not allocate memory, perform disk IO, or emit controller messages from
inside the callback.

Loss accounting should retain bounded attribution when cheap to collect:

- Dropped event count.
- First and last drop timestamps.
- Provider GUID.
- Event ID and version.
- Opcode, task, level, and keyword.
- Process/thread IDs when available.
- Event payload size.

When a free block becomes available again, the collector should emit a synthetic
health event describing the loss window before resuming normal telemetry output.
The synthetic event should flow through the same normalize, pack, durable queue,
and controller transport path as other collector health telemetry. Local
structured logging may additionally record rate-limited summaries for debugging,
but logs are secondary and must not be the only loss record.

An event that cannot fit into an empty block is an oversized-event case, not a
normal backpressure case. The collector should drop that event, account for it
separately, and emit health telemetry.

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

The collector should prefer explicit pipeline-level dropping over intentionally
stalling the ETW callback. If the callback blocks, ETW session buffers may fill
and the ETW runtime may drop events before the collector can attribute them.
Dropping after callback delivery allows the collector to preserve at least
bounded provider/event attribution for the blind spot.

## ETW Session Health Monitoring

`EtwHealthMonitor` should run outside the callback path. It can periodically
query the session with `ControlTrace(..., EVENT_TRACE_CONTROL_QUERY)` and
compare ETW-level loss with collector-level queue metrics.

The monitor should track deltas for ETW statistics such as:

- `EventsLost`
- `RealTimeBuffersLost`
- `LogBuffersLost`
- `NumberOfBuffers`
- `FreeBuffers`
- `BuffersWritten`

These counters distinguish failure modes:

- ETW loss increased, collector drop count did not: the session or consumer
  loop is not draining ETW quickly enough.
- Collector drop count increased, ETW loss did not: ETW is being drained, but
  decode/normalize/packing/durable queueing is saturated.
- Both increased: the complete telemetry pipeline is overloaded.

The monitor may use `ControlTrace(..., EVENT_TRACE_CONTROL_UPDATE)` to increase
runtime-adjustable session capacity such as `MaximumBuffers` within configured
limits. Per-buffer `BufferSize` is a session start-time property and should not
be treated as a normal runtime tuning knob.

Health queries should be adaptive:

- Normal state: query at a low rate, such as every 1-5 seconds.
- Pressure detected: temporarily query more frequently.
- Stable again: back off to the normal rate.

The callback path must not perform `ControlTrace` queries or updates.

## Threading Model

The initial threading model is:

```text
1 ETW consumer thread:
  ProcessTrace and callback snapshotting

N worker threads:
  decode, normalize, provider-specific enrichment, pack

1 durable queue writer:
  append packed records and maintain durable ordering/checkpoints

1 controller sender:
  send batches to sensor/controller and process acknowledgements

1 health monitor:
  query ETW and collector pressure metrics
```

Worker count should start from available CPU capacity but reserve threads for
ETW consumption, durable queue IO, controller transport, and service control.
The callback thread is dedicated to ETW consumption once it enters
`ProcessTrace`; it should not also be used as a decode/normalize worker.

# Controller Boundary

## Boundary

`sensor/controller` is the only sensor component that communicates with the
backend. OS-specific collectors such as `sensor/win32` communicate only with the
controller over local IPC.

## Rationale

This keeps platform collectors smaller, easier to reason about, and less exposed
to network-facing failure modes. It also makes offline collection and local
durability explicit.

## Collector Responsibilities

Collectors are responsible for:

- OS-specific telemetry collection.
- Source data validation.
- Normalization into shared schemas.
- Packing through an injectable packer interface.
- Durable local queueing before controller handoff.
- Local health and loss accounting.

Collectors are not responsible for:

- Internet connectivity.
- Backend authentication.
- Backend retry policy.
- Tenant routing.
- Cloud API protocol handling.

## Controller Responsibilities

The controller is responsible for:

- Receiving packed telemetry batches from collectors.
- Acknowledging accepted local batches.
- Applying backend throttling and retry behavior.
- Communicating with backend services.
- Managing backend authentication and authorization material.

## IPC Expectations

The collector-to-controller channel should support:

- Batching.
- Acknowledgement.
- Timeout and cancellation.
- Controller unavailable handling.
- Backpressure signaling.
- Version negotiation when the schema or packing format evolves.

The first Windows implementation should prefer a local Windows IPC mechanism
such as named pipes unless a stronger project constraint emerges.

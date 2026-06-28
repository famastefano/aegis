# Coding Boundaries

## Dependency Direction

Code should depend inward toward stable contracts:

```text
sensor/win32
  -> sensor/common

sensor/controller
  -> sensor/common
```

`sensor/common` must not depend on `sensor/win32` or `sensor/controller`.
`sensor/<os>` can't depend on `sensor/controller` and viceversa.

## Layer Ownership

- ETW session lifecycle belongs to the Windows collector.
- Provider configuration belongs near the Windows collector.
- Normalized schemas and shared telemetry contracts belong in `sensor/common`.
- Packing interfaces belong in `sensor/common`; concrete packers may live with
  the component that owns the format decision.
- Durable queue interfaces belong in `sensor/common` when shared, while the
  Windows implementation may live under `sensor/win32`.
- Backend communication belongs only to `sensor/controller`.

## Interface Boundaries

Prefer constructor-injected interfaces for components expected to change:

- Event decoding.
- Provider-specific adaptation.
- Telemetry normalization.
- Packing.
- Durable queue storage.
- Controller transport.
- Clocks and health reporting used by tests.

Avoid introducing a dependency injection framework.

## Testing Boundary

Repo-wide C++ unit-test policy is documented in
[`../../../docs/testing.md`](../../../docs/testing.md).

Sensor test targets should link the smallest sensor library under test, such as
`aegis_sensor_win32_core`, rather than the executable entry point.

Default unit tests should remain deterministic and should not require
administrator privileges, live ETW sessions, backend access, or network access.
Real ETW integration tests must stay separate from normal unit-test targets.

## Review Rules

Changes should be reviewed for:

- Security bypasses.
- Data loss or telemetry corruption.
- Callback blocking or unbounded memory growth.
- Queue corruption and crash recovery behavior.
- Schema compatibility.
- Sensitive data exposure in logs or persisted files.
- Hidden network access from `sensor/win32`.

Style concerns are secondary unless they weaken maintainability or violate the
established build and ownership boundaries.

# AGENTS.md

## Scope

These instructions apply to the standalone endpoint sensor project under
`sensor/`.

## Project Boundaries

- `sensor/win32` is the Windows endpoint telemetry collector.
- `sensor/win32` must not communicate with the internet or directly with the
  backend.
- `sensor/controller` is the cross-platform process that communicates with the
  backend and receives telemetry from OS-specific collectors.
- Shared contracts, schemas, packing interfaces, and IPC-neutral types belong in
  `sensor/common` unless they are truly platform-specific.
- Keep ETW collection, decoding, normalization, packing, durable storage, and
  controller transport as separate components.

## Win32 Collector Rules

- Use one ETW session for all enabled providers unless a documented security or
  reliability reason requires a separate session.
- Keep ETW callbacks minimal and non-blocking.
- Copy ETW-owned data before leaving the callback. Do not store borrowed
  pointers from `EVENT_RECORD`.
- Treat every ETW payload as untrusted and potentially malformed.
- Prefer provider-agnostic decoding first, with explicit provider-specific or
  event-specific adapters for exceptional cases.
- Preserve provenance, timestamps, provider identity, event identity, process
  identity, thread identity, activity identifiers, decode status, and loss
  accounting.
- Do not silently drop telemetry. If data is dropped, corrupted, throttled, or
  backpressured, emit health/loss telemetry when possible.
- Avoid logging sensitive endpoint data. Logs should support debugging without
  leaking secrets, credentials, tokens, private keys, or excessive personal data.

## Build And Dependencies

- Use target-scoped Modern CMake.
- Keep normal builds offline and deterministic.
- Prefer STL first, Win32 APIs second, and third-party dependencies only when
  justified.
- Vendored dependencies belong in `thirdparty/<dep-name>` with license,
  version metadata, and REASON.md where we explain why brought the dependency,
  rather than rolling our own.
- Do not add dependencies from the network as part of configure or build.
- MSVC is the native compiler for the Windows collector. Presets assume the MSVC
  environment has already been initialized.

## Testing Expectations

- Add focused tests for schema changes, parsers, malformed events, queue
  recovery, backpressure, corruption handling, and controller IPC behavior.
- Do not require administrator privileges for default unit tests.
- Keep real ETW integration tests separate from deterministic unit tests.
- Use inert synthetic fixtures for security-sensitive behavior.

## Design Documentation

- Update `docs/architecture/` when changing component boundaries or data flow.
- Add or update an ADR under `docs/adr/` for durable architectural decisions.
- Schema semantics, packing changes, and durable queue behavior require
  documentation and tests.

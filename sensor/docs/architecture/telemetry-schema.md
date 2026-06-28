# Telemetry Schema

## Schema Shape

Normalized telemetry should use a stable envelope with a typed body and a
source-specific extension area:

```text
NormalizedEvent
  schema_version
  event_id
  event_category
  observed_at
  source
  host
  process
  thread
  user
  correlation
  confidence
  body
  source_extensions
  collection_status
```

The exact representation is intentionally deferred until implementation, but the
boundary is fixed: ETW source details should not leak into every consumer-facing
field, and generic source details should not be discarded.

## Required Semantics

The schema should preserve:

- Event source and provider identity.
- Provider event ID, version, level, opcode, task, and keywords.
- Timestamps and timestamp source.
- Process and thread identity.
- Activity and related activity identifiers.
- Decode status and parser errors.
- Collection status, loss accounting, and queue pressure.
- Stable schema version.

## Source Extensions

`source_extensions` may carry ETW-specific or provider-specific data that does
not yet belong in a stable normalized field. This keeps the schema flexible
without forcing backend consumers to depend on raw ETW layouts.

## Versioning

Schema changes require a version update when field meaning changes. Additive
fields may be introduced without breaking older consumers if defaults and
absence semantics are documented.

## Privacy

New fields must be reviewed for privacy and retention impact. Do not add fields
that expose credentials, secrets, private keys, tokens, or unnecessary personal
data.

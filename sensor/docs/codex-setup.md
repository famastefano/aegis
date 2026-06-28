# Codex Setup

## Files Codex Should Read

Future Codex work should use these files as the project memory:

- `AGENTS.md` at the repository root for global engineering standards.
- `docs/architecture` for repository-wide boundaries.
- `sensor/AGENTS.md` for sensor-specific boundaries and safety rules.
- `sensor/docs/architecture/` for component boundaries and data flow.
- `sensor/docs/adr/` for accepted architectural decisions.
- `sensor/docs/build.md` for local build expectations.

This is preferable to relying on chat history. Design decisions that should
survive across sessions belong in versioned files.

## Skills

No custom Codex skill is needed yet. The project is better served by committed
repository guidance while the architecture is still forming.

Consider a custom skill later if repeated work becomes highly procedural, such
as generating new telemetry schema files, adding provider adapters with matching
tests, or preparing release artifacts.

## How To Guide Code Generation

Define behavior in three layers:

- `AGENTS.md` files for hard rules and coding standards.
- Architecture docs for component boundaries, ownership, and rationale.
- Tests for executable contracts and regression protection.

When Codex generates code, it should preserve dependency direction, keep changes
small, add tests for meaningful behavior, and update architecture or ADR files
when design intent changes.

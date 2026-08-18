# Changelog

## [Unreleased]

### Added
- Optional `instid` parameter to `get_wave()`, `get_msg()`, and `copymsg_type()`. Pass `instid=0` for wildcard (all installations) or a specific ID to filter. Default behavior is unchanged.
- `instid` and `modid` fields to the dictionary returned by `EWModule.get_wave()`, exposing the source module and installation IDs from the message logo.
- `demo_getwave.py` example script showing continuous waveform reading with `EWModule.get_wave()`.
- `docker-compose.yaml` for running the container with bind-mounted params and demo scripts.
- Dockerfile for building PyEarthworm using pre-compiled Earthworm v8.0b8 binaries on Rocky Linux 9.6 (no source compilation required).
`test/earthworm/` directory with Earthworm runtime configuration (params, bin scripts) for containerized testing.
- `CHANGELOG.md` to track project changes.

### Fixed
- PID comparison in `stopThread` and `restartThread` now strips null bytes and whitespace, extracts digits only, and uses exact equality instead of substring matching. Prevents false-positive PID matches (e.g., PID 12 matching inside "1234").

### Changed
- Updated README to reflect new parameters and return values.
- Updated `.gitignore` to ignore `.vscode/`, `*.state` files, and test directory paths.

## [1.41] - Previous release

See README.md for prior history.

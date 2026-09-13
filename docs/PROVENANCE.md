# Provenance

The UI engine in this repository started as a hard fork of [RmlUi](https://github.com/mikke89/RmlUi) **6.2**
(commit `2230d1a6e8e0848ed87a5761e2a5160b2a175ba4`).

As of the first-party layout redesign ([ADR 001](ADR_001_FIRST_PARTY_LAYOUT.md)), that history is **provenance only**:

- Sources live under `include/RmlUi/` and `src/{core,svg,debugger}/` as owned code.
- We do **not** preserve an upstream-shaped `rmlui/` tree for re-import.
- Useful upstream fixes may be cherry-picked; there is no expectation of merging full upstream releases.

License: MIT — see `LICENSE` and historical notices in engine headers.

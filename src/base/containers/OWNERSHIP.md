# Owned container implementations

These headers are **first-party pins** of third-party libraries. Prefer the
`ui::UnorderedMap` / `ui::UnorderedSet` / `Small*` aliases in
`include/ui/config/Config.h` — do not spread `robin_hood::` or `itlib::` types
through public headers.

| File | Upstream | Why pinned |
|------|----------|------------|
| `robin_hood.h` | martinus/robin-hood-hashing (MIT), archived upstream | Fast flat hash maps used by default `UnorderedMap`/`UnorderedSet` |
| `itlib/flat_map.hpp`, `flat_set.hpp` | itlib (BSL-1.0) | Small ordered maps/sets for `SmallUnorderedMap` etc. |

To use STL instead, define `UI_NO_THIRDPARTY_CONTAINERS`.

Update policy: bump deliberately with a short note here; keep behind the Config aliases.

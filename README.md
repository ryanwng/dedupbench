# MaxCDC and RepMaxCDC for DedupBench

`maxcdc-cpp-port.patch` adds two chunking algorithms from E. C. Schouten,
*Content-Defined Chunking with tight chunk size bounds* (2026), to DedupBench. They are C++ ports of
[go-cdc](https://github.com/buildbarn/go-cdc) (Apache-2.0) and give exactly the same chunks.

| Name in config | Algorithm |
| --- | --- |
| `maxcdc` | MaxCDC |
| `repmaxcdc` | RepMaxCDC (RecMaxCDC = RepMaxCDC with `horizon = buffer_size − 2·min`) |

## What the patch changes

The algorithms are added the same way as the upstream ones (e.g. TTTD):

- **New files:** `dedup/{include,src}/chunking/maxcdc_chunking.*` and `repmaxcdc_chunking.*`
- **Registration:** a few lines in `dedup/include/config/config.hpp`, `dedup/src/config/config.cpp`
  and `dedup/src/driver.cpp`
- **Example configs:** `build/config_maxcdc_8kb/`

## Apply and build

From the root of a DedupBench checkout (made against commit `8e2697c`):

```bash
git apply --check maxcdc-cpp-port.patch   # dry run
git apply maxcdc-cpp-port.patch
cd build && make clean && make
```

To remove it again: `git apply -R maxcdc-cpp-port.patch`.

## Configure

```
chunking_algo=repmaxcdc
hashing_algo=xxhash128
output_file=./hashes/repmaxcdc_8kb.out
simd_mode=none
buffer_size=131072

repmaxcdc_min_block_size=6124      # average ≈ min / 0.74759 ≈ 8 KiB
repmaxcdc_horizon_size=48992       # 8 × min
```

For MaxCDC, set `maxcdc_min_block_size` and `maxcdc_max_block_size` instead (≈ 8 KiB average:
4670 and 11675).

**`buffer_size` must be large enough:** at least `min + max` for MaxCDC and `2·min + horizon` for
RepMaxCDC. DedupBench's usual 32768 is too small at 8 KiB. If it's too small, the program stops with
an error. Use the same `buffer_size` for every algorithm in a comparison.

Then run as usual from `build/`, e.g. with the example configs:

```bash
./dedup_script.sh -c maxcdc_8kb <dataset_folder>
```

## Notes

- This is go-cdc's *simple* version: same chunks, but RepMaxCDC is slower than go-cdc's optimised
  version.
- `maxcdc-part1/scripts/check_cpp_port.sh` checks the chunks against go-cdc (needs the Go harness
  from `maxcdc-part1`).

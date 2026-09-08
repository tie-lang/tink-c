# tink-c

tink data-flow node frame protocol — C99 library (single header + source, no
dependencies). Universal and language-agnostic: any component that obeys the
frame protocol can join a tink pipeline.

```
帧 = [ len: u32 BE ][ payload: len 字节 ][ crc: u32 BE ]
len = payload 字节数
crc = CRC32-IEEE(payload)（多项式 0xEDB88320）
```

Mirrors `std/tink.tie` (tie standard library) and the Rust / Python tink
libraries; pure functions over byte buffers, IO (stdin/stdout) left to the
caller. `frame_next` / `frame_skip` are zero-copy (return pointers into the
input buffer).

## API

| function | description |
| --- | --- |
| `tink_crc32(data, len) -> uint32_t` | CRC32-IEEE over a byte buffer. Check vector: `tink_crc32("123456789", 9) == 0xCBF43926` |
| `tink_frame_encode(payload, payload_len, out) -> size_t` | encode a payload into a full frame `[len][payload][crc]`; `out` needs `payload_len + 8` capacity |
| `tink_frame_next(bytes, bytes_len, pos, &payload, &payload_len, &next_pos) -> int` | parse one frame at `pos`, verify CRC; returns `1` on success, `0` on failure |
| `tink_frame_skip(bytes, bytes_len, pos, &next_pos) -> int` | skip one frame at `pos` without copying or verifying; `1` on success, `0` on failure |

## Usage

```c
#include "tink.h"

uint8_t buf[8 + 3];
size_t n = tink_frame_encode((const uint8_t *)"\x01\x02\x03", 3, buf);

const uint8_t *p; size_t plen, next;
if (tink_frame_next(buf, n, 0, &p, &plen, &next) == 1) {
    /* p points into buf, plen == 3, next == n */
}
```

## Build & test

```bash
clang -std=c99 -Wall -Wextra -o test_tink test_tink.c tink.c && ./test_tink
```

## Cross-language

tink 帧协议各语言实现（API 语义与校验向量一致）：

| language | library |
| --- | --- |
| tie | `std/tink.tie` |
| Rust | `tink-rust`（tink crate） |
| C | this library（`tink-c`） |
| Python | `tink-python`（`tink.py`） |

## License

本仓库使用 **Tie Public License v1.2 (TPL 1.2)**，完整文本见 [LICENSE](LICENSE)。
This repository is distributed under the **Tie Public License v1.2 (TPL 1.2)** — see [LICENSE](LICENSE) for the full text.

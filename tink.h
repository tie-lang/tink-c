/* tink.h —— tink data-flow node frame protocol (universal, language-agnostic).
 *
 * Frame = [len u32 BE][payload][crc u32 BE]; crc = CRC32-IEEE (0xEDB88320).
 * Mirrors std/tink.tie (tie standard library) and the Rust crate / Python
 * package; pure functions over byte buffers, IO (stdin/stdout) left to the
 * caller. C99. No dependencies.
 */
#ifndef TINK_H
#define TINK_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CRC32-IEEE over a byte buffer (bit-loop, no table; matches zlib.crc32).
 * Check vector: tink_crc32("123456789", 9) == 0xCBF43926. */
uint32_t tink_crc32(const uint8_t *data, size_t len);

/* Encode payload into a full frame written to `out`.
 * Frame is exactly `payload_len + 8` bytes; `out` must have that capacity.
 * Returns bytes written (payload_len + 8) on success, 0 on failure. */
size_t tink_frame_encode(const uint8_t *payload, size_t payload_len,
                         uint8_t *out);

/* Parse one frame at `pos` (verifies CRC, zero-copy).
 * On success sets *payload (into `bytes`), *payload_len, *next_pos and
 * returns 1. On out-of-bounds or CRC mismatch returns 0. */
int tink_frame_next(const uint8_t *bytes, size_t bytes_len, size_t pos,
                    const uint8_t **payload, size_t *payload_len,
                    size_t *next_pos);

/* Skip one frame at `pos` without copying or verifying (zero-copy).
 * On success sets *next_pos and returns 1; on out-of-bounds returns 0. */
int tink_frame_skip(const uint8_t *bytes, size_t bytes_len, size_t pos,
                    size_t *next_pos);

#ifdef __cplusplus
}
#endif

#endif /* TINK_H */

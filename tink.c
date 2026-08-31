/* tink.c —— implementation of tink.h (tink data-flow node frame protocol). */
#include "tink.h"

uint32_t tink_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = UINT32_C(0xFFFFFFFF);
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (UINT32_C(0xEDB88320) & (0u - (crc & 1u)));
    }
    return crc ^ UINT32_C(0xFFFFFFFF);
}

static int read_be32(const uint8_t *b, uint32_t *v) {
    *v = ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) |
         ((uint32_t)b[2] << 8) | (uint32_t)b[3];
    return 1;
}

size_t tink_frame_encode(const uint8_t *payload, size_t payload_len,
                         uint8_t *out) {
    if (!out)
        return 0;
    uint32_t n = (uint32_t)payload_len;
    out[0] = (uint8_t)(n >> 24);
    out[1] = (uint8_t)(n >> 16);
    out[2] = (uint8_t)(n >> 8);
    out[3] = (uint8_t)n;
    if (payload_len)
        for (size_t i = 0; i < payload_len; i++)
            out[4 + i] = payload[i];
    uint32_t c = tink_crc32(payload, payload_len);
    out[4 + payload_len] = (uint8_t)(c >> 24);
    out[4 + payload_len + 1] = (uint8_t)(c >> 16);
    out[4 + payload_len + 2] = (uint8_t)(c >> 8);
    out[4 + payload_len + 3] = (uint8_t)c;
    return payload_len + 8;
}

int tink_frame_next(const uint8_t *bytes, size_t bytes_len, size_t pos,
                    const uint8_t **payload, size_t *payload_len,
                    size_t *next_pos) {
    if (bytes_len < pos + 8)
        return 0;
    uint32_t n;
    read_be32(bytes + pos, &n);
    size_t end = pos + 8 + (size_t)n;
    if (bytes_len < end)
        return 0;
    uint32_t want = ((uint32_t)bytes[end - 4] << 24) |
                    ((uint32_t)bytes[end - 3] << 16) |
                    ((uint32_t)bytes[end - 2] << 8) | (uint32_t)bytes[end - 1];
    if (tink_crc32(bytes + pos + 4, (size_t)n) != want)
        return 0;
    *payload = bytes + pos + 4;
    *payload_len = (size_t)n;
    *next_pos = end;
    return 1;
}

int tink_frame_skip(const uint8_t *bytes, size_t bytes_len, size_t pos,
                    size_t *next_pos) {
    if (bytes_len < pos + 8)
        return 0;
    uint32_t n;
    read_be32(bytes + pos, &n);
    size_t end = pos + 8 + (size_t)n;
    if (bytes_len < end)
        return 0;
    *next_pos = end;
    return 1;
}

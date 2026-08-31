/* test_tink.c —— unit tests for the C tink library (tink.h / tink.c).
 * Build: clang test_tink.c tink.c -o test_tink && ./test_tink  (or cc)  */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tink.h"

static int failures = 0;
#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
            failures++;                                                    \
        }                                                                  \
    } while (0)

static void test_crc32_vector(void) {
    CHECK(tink_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u);
    CHECK(tink_crc32((const uint8_t *)"", 0) == 0u);
}

static void test_frame_roundtrip(void) {
    const uint8_t payload[] = {1, 2, 3};
    uint8_t frame[8 + sizeof(payload)];
    size_t n = tink_frame_encode(payload, sizeof(payload), frame);
    CHECK(n == sizeof(payload) + 8);

    const uint8_t *p;
    size_t plen, next;
    CHECK(tink_frame_next(frame, n, 0, &p, &plen, &next) == 1);
    CHECK(next == n);
    CHECK(plen == sizeof(payload));
    CHECK(memcmp(p, payload, sizeof(payload)) == 0);
}

static void test_empty_frame_roundtrip(void) {
    uint8_t frame[8];
    size_t n = tink_frame_encode(NULL, 0, frame);
    CHECK(n == 8);
    const uint8_t *p;
    size_t plen, next;
    CHECK(tink_frame_next(frame, n, 0, &p, &plen, &next) == 1);
    CHECK(next == n);
    CHECK(plen == 0);
}

static void test_crc_tamper_rejected(void) {
    uint8_t frame[8 + 3];
    size_t n = tink_frame_encode((const uint8_t *)"\x01\x02\x03", 3, frame);
    frame[5]++; /* tamper payload[1] */
    const uint8_t *p;
    size_t plen, next;
    CHECK(tink_frame_next(frame, n, 0, &p, &plen, &next) == 0);
}

static void test_frame_skip_matches_len(void) {
    const uint8_t payload[] = {1, 2, 3};
    uint8_t frame[8 + sizeof(payload)];
    size_t n = tink_frame_encode(payload, sizeof(payload), frame);
    size_t next;
    CHECK(tink_frame_skip(frame, n, 0, &next) == 1);
    CHECK(next == n);
}

static void test_out_of_bounds(void) {
    uint8_t frame[8 + 3];
    size_t n = tink_frame_encode((const uint8_t *)"\x01\x02\x03", 3, frame);
    const uint8_t *p;
    size_t plen, next;
    CHECK(tink_frame_next(frame, n, n, &p, &plen, &next) == 0);
    size_t skipped;
    CHECK(tink_frame_skip(frame, n, n, &skipped) == 0);
    CHECK(tink_frame_next(frame, n - 1, 0, &p, &plen, &next) == 0);
}

static void test_multiple_frames(void) {
    /* encode two frames back-to-back; both parse in sequence */
    uint8_t a[8 + 2], b[8 + 1];
    size_t na = tink_frame_encode((const uint8_t *)"\x0A\x0B", 2, a);
    size_t nb = tink_frame_encode((const uint8_t *)"\x63", 1, b);
    uint8_t buf[64];
    memcpy(buf, a, na);
    memcpy(buf + na, b, nb);

    const uint8_t *p;
    size_t plen, next;
    CHECK(tink_frame_next(buf, na + nb, 0, &p, &plen, &next) == 1);
    CHECK(plen == 2 && p[0] == 0x0A && p[1] == 0x0B);
    size_t pos2 = next;
    CHECK(tink_frame_next(buf, na + nb, pos2, &p, &plen, &next) == 1);
    CHECK(plen == 1 && p[0] == 0x63);
    CHECK(next == na + nb);
}

int main(void) {
    test_crc32_vector();
    test_frame_roundtrip();
    test_empty_frame_roundtrip();
    test_crc_tamper_rejected();
    test_frame_skip_matches_len();
    test_out_of_bounds();
    test_multiple_frames();
    if (failures) {
        printf("%d test(s) FAILED\n", failures);
        return 1;
    }
    printf("all C tink tests passed\n");
    return 0;
}

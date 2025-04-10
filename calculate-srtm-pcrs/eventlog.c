/* SPDX-License-Identifier: BSD-2-Clause-Patent */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "eventlog.h"
#include "hash.h"

static char *
encode_hex(const uint8_t *bin, int length)
{
    size_t len = length * 2 + 1;
    char *hex = calloc(len, 1);
    for (int i = 0; i < length; ++i) {
        // snprintf writes a '0' byte
        snprintf(hex + i * 2, 3, "%.2x", bin[i]);
    }
    return hex;
}

int
evlog_add(eventlog_t *evlog, uint32_t pcr_index, const char *name, uint8_t *hash, const char *desc)
{
    int ret;
    char *hashstr = encode_hex(hash, SHA256_DIGEST_LENGTH);
    if (!hashstr) {
        printf("Failed to allocate memory\n");
        return -1;
    }

    char s[4096] = { 0 };
    if (evlog->format == FORMAT_JSON) {
        ret = snprintf(s, sizeof(s),
                       "{"
                       "\n\t\"type\":\"TPM Reference Value\","
                       "\n\t\"subtype\":\"%s\","
                       "\n\t\"index\":%d,"
                       "\n\t\"sha256\":\"%s\","
                       "\n\t\"description\":\"PCR%d: %s\""
                       "\n},\n",
                       name, pcr_index, hashstr, pcr_index, desc);
    } else if (evlog->format == FORMAT_TEXT) {
        ret = snprintf(s, sizeof(s),
                       "name: %s"
                       "\n\tpcr: %d"
                       "\n\tsha256: %s"
                       "\n\tdescription: %s\n",
                       name, pcr_index, hashstr, desc);
    }
    if (!ret) {
        printf("Failed to print eventlog\n");
        ret = -1;
        goto out;
    }

    if (!evlog->log[pcr_index]) {
        size_t size = strlen(s) + 1;
        evlog->log[pcr_index] = (char *)malloc(size);
        if (!evlog->log[pcr_index]) {
            printf("Failed to allocate memory\n");
            ret = -1;
            goto out;
        }
        strncpy(evlog->log[pcr_index], s, size);
    } else {
        size_t size = strlen(evlog->log[pcr_index]) + strlen(s) + 1;
        evlog->log[pcr_index] = (char *)realloc(evlog->log[pcr_index], size);
        if (!evlog->log[pcr_index]) {
            printf("Failed to allocate memory\n");
            ret = -1;
            goto out;
        }
        strncat(evlog->log[pcr_index], s, strlen(s) + 1);
    }

out:
    free(hashstr);
    return ret;
}
/*
 * Copyright (c) International Business Machines  Corp., 2008
 * Copyright (C) 2013 Politecnico di Torino, Italy
 *                    TORSEC group -- http://security.polito.it
 *
 * Copyright(c) 2021 Fraunhofer AISEC
 * Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V.
 *
 * Authors:
 * Reiner Sailer <sailer@watson.ibm.com>
 * Mimi Zohar <zohar@us.ibm.com>
 * Roberto Sassu <roberto.sassu@polito.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * File: ima_measure.c
 *
 * Calculate the SHA1 aggregate-pcr value based on the IMA runtime
 * binary measurements.
 *
 * The file was modified to support the ima-modsig template and
 * stripped of components not required for this project. Furthermore,
 * the file outputs the measurements in json format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>

#include <openssl/pkcs7.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#include "common.h"
#include "hash.h"
#include "modsig.h"
#include "ima_measure.h"

#define TCG_EVENT_NAME_LEN_MAX 255
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
/*
 * IMA template descriptor definition
 */
typedef struct {
    char *name;
    char *fmt;
    int num_fields;
    struct ima_template_field **fields;
} ima_template_desc_t;

/*
 * signature format v2 - for using with asymmetric keys - taken from kernel sources
 */
#pragma pack(push, 1)
struct signature_v2_hdr {
    uint8_t type;      /* xattr type */
    uint8_t version;   /* signature format version */
    uint8_t hash_algo; /* Digest algorithm [enum hash_algo] */
    uint32_t keyid;    /* IMA key identifier - not X509/PGP specific */
    uint16_t sig_size; /* signature size */
    uint8_t sig[];     /* signature payload */
} __packed;
#pragma pack(pop)

/*
 * IMA binary_runtime_measurements event entry
 */
struct event {
    struct {
        int32_t pcr;
        uint8_t digest[SHA_DIGEST_LENGTH];
        uint32_t name_len;
    } header;
    char name[TCG_EVENT_NAME_LEN_MAX + 1];
    ima_template_desc_t *ima_template_desc;
    uint32_t template_data_len;
    uint8_t *template_data;
};

// Known IMA template descriptors, see https://docs.kernel.org/security/IMA-templates.html
static ima_template_desc_t ima_template_desc[] = { { .name = "ima", .fmt = "d|n" },
                                                   { .name = "ima-ng", .fmt = "d-ng|n-ng" },
                                                   { .name = "ima-sig", .fmt = "d-ng|n-ng|sig" },
                                                   { .name = "ima-modsig",
                                                     .fmt = "d-ng|n-ng|sig|d-modsig|modsig" } };

static char *
get_module_name(uint8_t *buffer, size_t len)
{
    // Ensure terminating null
    char *str = calloc(1, len + 1);
    if (!str) {
        printf("Failed to allocate memory for module name");
        return NULL;
    }
    memcpy(str, buffer, len);
    return str;
}

static int
buf_read(void *dest, uint8_t **ptr, size_t size, size_t *remain)
{
    if (size > *remain) {
        return -1;
    }

    memcpy(dest, *ptr, size);
    *ptr += size;
    *remain -= size;
    return 0;
}

static char *
get_template_data(struct event *template)
{
    int offset = 0;
    size_t i;
    int is_ima_template;
    char *template_fmt, *template_fmt_ptr, *f;
    uint32_t digest_len = 0;
    uint8_t *digest = NULL;

    is_ima_template = strcmp(template->name, "ima") == 0 ? 1 : 0;
    template->ima_template_desc = NULL;

    for (i = 0; i < ARRAY_SIZE(ima_template_desc); i++) {
        if (strcmp(template->name, ima_template_desc[i].name) == 0) {
            template->ima_template_desc = ima_template_desc + i;
            break;
        }
    }

    if (template->ima_template_desc == NULL) {
        i = ARRAY_SIZE(ima_template_desc) - 1;
        template->ima_template_desc = ima_template_desc + i;
        template->ima_template_desc->fmt = template->name;
    }

    template_fmt = strdup(template->ima_template_desc->fmt);
    if (!template_fmt) {
        printf("Failed to allocate memory for template_fmt");
        return NULL;
    }

    template_fmt_ptr = template_fmt;
    for (i = 0; (f = strsep(&template_fmt_ptr, "|")) != NULL; i++) {
        uint32_t field_len = 0;

        DEBUG("field is: %s", f);

        if (is_ima_template && strcmp(f, "d") == 0)
            field_len = SHA_DIGEST_LENGTH;
        else if (is_ima_template && strcmp(f, "n") == 0)
            field_len = strlen((const char *)template->template_data + offset);
        else {
            memcpy(&field_len, template->template_data + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }

        if (strncmp(template->name, "ima-sig", 7) == 0) {
            if (strncmp(f, "d-ng", 4) == 0) {
                int algo_len = strlen((char *)template->template_data + offset) + 1;

                digest = template->template_data + offset + algo_len;
                digest_len = field_len - algo_len;

            } else if (strncmp(f, "sig", 3) == 0) {
                print_data_no_lf(digest, digest_len, "Digest");

                if (template->template_data_len <= (uint32_t)offset) {
                    printf("%s: No signature present", f);
                    continue;
                }

                uint8_t *field_buf = template->template_data + offset;

                if (*field_buf != 0x03) {
                    printf("%s: No signature present", f);
                    continue;
                }

                struct signature_v2_hdr *sig =
                    (struct signature_v2_hdr *)(template->template_data + offset);

                print_data_no_lf(sig->sig, field_len - sizeof(struct signature_v2_hdr),
                                 "Signature");

            } else if (strncmp(f, "n-ng", 4) == 0) {
                free(template_fmt);
                return get_module_name(template->template_data + offset, field_len);
            }

        } else if (strncmp(template->name, "ima-modsig", 10) == 0) {
            if (strncmp(f, "d-ng", 4) == 0) {
                DEBUG("Field %s not evaluated at the moment", f);

            } else if (strncmp(f, "d-modsig", 8) == 0) {
                int algo_len = strlen((char *)template->template_data + offset) + 1;
                digest = template->template_data + offset + algo_len;
                digest_len = field_len - algo_len;

            } else if (strcmp(f, "modsig") == 0) {
                sig_info_t *sig_info = NULL;
                sig_info =
                    modsig_parse_new((const char *)(template->template_data + offset), field_len);
                if (!sig_info) {
                    printf("Failed to parse module signature");
                    continue;
                }

                print_data_no_lf(sig_info->sig, sig_info->sig_len, "Signature");

                modsig_free(sig_info);

            } else if (strncmp(f, "n-ng", 4) == 0) {
                free(template_fmt);
                return get_module_name(template->template_data + offset, field_len);
            }
        } else if (strncmp(template->name, "ima-ng", 6) == 0) {
            if (strncmp(f, "d-ng", 4) == 0) {
                int algo_len = strlen((char *)template->template_data + offset) + 1;

                digest = template->template_data + offset + algo_len;
                digest_len = field_len - algo_len;

            } else if (strncmp(f, "n-ng", 4) == 0) {
                free(template_fmt);
                return get_module_name(template->template_data + offset, field_len);
            }
        } else {
            printf("Unknown IMA template %s", template->name);
        }

        offset += field_len;
    }

    free(template_fmt);
    return NULL;
}

static int
read_template_data(struct event *template, uint8_t **buf, size_t *remain)
{
    int len, is_ima_template;

    is_ima_template = strcmp(template->name, "ima") == 0 ? 1 : 0;
    if (!is_ima_template) {
        buf_read(&template->template_data_len, buf, sizeof(uint32_t), remain);
        len = template->template_data_len;
    } else {
        template->template_data_len = SHA_DIGEST_LENGTH + TCG_EVENT_NAME_LEN_MAX + 1;
        /*
		 * Read the digest only as the event name length
		 * is not known in advance.
		 */
        len = SHA_DIGEST_LENGTH;
    }

    template->template_data = calloc(template->template_data_len, sizeof(uint8_t));
    if (!template->template_data) {
        printf("Failed to allocate memory for template data\n");
    }

    buf_read(template->template_data, buf, len, remain);
    if (is_ima_template) { /* finish 'ima' template data read */
        uint32_t field_len = 0;

        buf_read(&field_len, buf, sizeof(uint32_t), remain);
        buf_read(template->template_data + SHA_DIGEST_LENGTH, buf, field_len, remain);
    }

    return 0;
}

static int
verify_template_hash(struct event *template)
{
    uint8_t digest[SHA_DIGEST_LENGTH];

    hash_buf(EVP_sha1(), digest, template->template_data, template->template_data_len);
    if (memcmp(digest, template->header.digest, sizeof(digest)) == 0) {
        return 0;
    }
    return -1;
}

int
ima_parse_binary_runtime_measurements(uint8_t *buf, size_t size)
{
    ASSERT(buf);

    struct event template;
    uint8_t *ptr = buf;
    size_t remain = size;
    int ret = 0;

    DEBUG("Parsing IMA runtime measurements, bufsize %lu..", size);

    bool first = true;
    while (!buf_read(&template.header, &ptr, sizeof(template.header), &remain)) {
        DEBUG("PCR %02d Measurement:", template.header.pcr);

        if (template.header.name_len > TCG_EVENT_NAME_LEN_MAX) {
            printf("template header name too long: %d\n", template.header.name_len);
            free(template.template_data);
            ret = -1;
            goto clean;
        }

        memset(template.name, 0, sizeof(template.name));
        buf_read(template.name, &ptr, template.header.name_len, &remain);
        DEBUG("Template: %s", template.name);

        if (read_template_data(&template, &ptr, &remain) < 0) {
            printf("Failed to read measurement entry %s", template.name);
            ret = -1;
            free(template.template_data);
            goto clean;
        }

        if (verify_template_hash(&template) != 0) {
            printf("Failed to verify template hash for %s", template.name);
            ret = -1;
            free(template.template_data);
            goto clean;
        }

        char *module_name = get_template_data(&template);
        if (!module_name) {
            printf("Failed to parse measurement entry %s", template.name);
            ret = -1;
            free(template.template_data);
            goto clean;
        }

        uint8_t template_hash[SHA256_DIGEST_LENGTH];
        hash_buf(EVP_sha256(), template_hash, template.template_data, template.template_data_len);
        char *h = convert_bin_to_hex(template_hash, SHA256_DIGEST_LENGTH);

        char *tmp = strdup(module_name);
        char *name = basename(tmp);

        if (first) {
            first = false;
            printf(
                "[{\n\t\"type\":\"TPM Reference Value\",\n\t\"subtype\":\"%s\",\n\t\"index\":10,\n\t\"sha256\":\"%s\",\n\t\"description\":\"%s\",\n\t\"optional\":true\n}",
                name, h, module_name);
        } else {
            printf(
                ",\n{\n\t\"type\":\"TPM Reference Value\",\n\t\"subtype\":\"%s\",\n\t\"index\":10,\n\t\"sha256\":\"%s\",\n\t\"description\":\"%s\",\n\t\"optional\":true\n}",
                name, h, module_name);
        }

        free(h);
        free(module_name);
        free(tmp);
        free(template.template_data);
    }

    printf("]\n");

clean:
    return ret;
}

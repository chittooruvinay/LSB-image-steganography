#ifndef DECODE_H
#define DECODE_H

#include "types.h"  // For Status, etc.

#define MAGIC_STRING "#*"  // Must match your encode magic string

typedef struct _DecodeInfo
{
    /* Source Stego Image */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output File */
    char output_fname[100];
    FILE *fptr_output;

    /* Data extracted */
    char extn_secret_file[8];
    long size_secret_file;
} DecodeInfo;

/* Function Prototypes */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *size);

Status decode_secret_file_extn(DecodeInfo *decInfo, int size);

Status decode_secret_file_size(DecodeInfo *decInfo, long *size);

Status decode_secret_file_data(DecodeInfo *decInfo, long size);

char decode_byte_from_lsb(char *image_buffer);

int decode_size_from_lsb(char *image_buffer);

#endif

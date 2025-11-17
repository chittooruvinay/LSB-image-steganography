#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    printf("Read and validate decode arguments started...\n");

    // Step 1: Validate stego image file (.bmp)
    if (argv[2] == NULL)
    {
        printf("INVALID: Missing stego image file.\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("INVALID: Stego image must have a .bmp extension.\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    // Step 2: Handle output filename (optional)
    if (argv[3] != NULL)
    {
        static char outputBuffer[100];
        strcpy(outputBuffer, argv[3]);

        char *dot = strrchr(outputBuffer, '.');

        // Check if extension exists
        if (dot == NULL)
        {
            printf("INVALID: The output file must have a valid extension (.txt, .c, or .sh)\n");
            return e_failure;
        }

        // Check for valid extensions
        if (strcmp(dot, ".txt") == 0 || strcmp(dot, ".c") == 0 || strcmp(dot, ".sh") == 0)
        {
            // Remove extension safely and store filename
            *dot = '\0';
            strcpy(decInfo->output_fname, outputBuffer);
            printf("Valid output file extension detected: %s\n", dot + 1);
            printf("Output base filename set to '%s'\n", decInfo->output_fname);
        }
        else
        {
            printf("INVALID: Unsupported file extension '%s'.\n", dot);
            printf("Please provide a file with one of the following extensions: .txt, .c, or .sh\n");
            return e_failure;
        }
    }
    else
    {
        printf("No output filename provided. Using default: 'decoded'\n");
        strcpy(decInfo->output_fname, "decoded");
    }

    return e_success;
}


/* Open only stego file now (output will be opened later) */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "INVALID: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    printf("Stego image opened successfully: %s\n", decInfo->stego_image_fname);
    return e_success;
}

/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header

    char image_buffer[8];
    char decoded_char;
    char decoded_magic[10] = {0};

    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        decoded_char = decode_byte_from_lsb(image_buffer);
        decoded_magic[i] = decoded_char;
    }

    decoded_magic[strlen(magic_string)] = '\0';

    if (strcmp(decoded_magic, magic_string) == 0)
    {
        printf("Magic string verified successfully: %s\n", decoded_magic);
        return e_success;
    }
    else
    {
        printf("INVALID: Magic string mismatch. Not a valid stego image.\n");
        return e_failure;
    }
}

/* Decode Extension Size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *size)
{
    char image_buffer[32];
    fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image);
    *size = decode_size_from_lsb(image_buffer);
    printf("Decoded secret file extension size = %d\n", *size);
    return e_success;
}

/* Decode Extension */
Status decode_secret_file_extn(DecodeInfo *decInfo, int size)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);
    }
    decInfo->extn_secret_file[size] = '\0';
    printf("Decoded extension = %s\n", decInfo->extn_secret_file);

    // Append extension to output filename
    strcat(decInfo->output_fname, decInfo->extn_secret_file);

    // Open output file now
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "INVALID: Unable to open output file %s\n", decInfo->output_fname);
        return e_failure;
    }

    printf("Output file created: %s\n", decInfo->output_fname);
    return e_success;
}

/* Decode Secret File Size */
Status decode_secret_file_size(DecodeInfo *decInfo, long *size)
{
    char image_buffer[32];
    fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image);
    *size = decode_size_from_lsb(image_buffer);
    decInfo->size_secret_file = *size;
    printf("Decoded secret file size = %ld bytes\n", *size);
    return e_success;
}

/* Decode Secret File Data */
Status decode_secret_file_data(DecodeInfo *decInfo, long size)
{
    char image_buffer[8];
    char ch;

    printf("Decoding secret file data...\n");
    for (long i = 0; i < size; i++)
    {
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        ch = decode_byte_from_lsb(image_buffer);
        fputc(ch, decInfo->fptr_output);
    }
    printf("Decoded secret data successfully.\n");
    return e_success;
}

/* Utility Functions */
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = (data << 1) | (image_buffer[i] & 1);
    }
    return data;
}

int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);
    }
    return size;
}

/* Main Decoding Orchestrator */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("<----------- DECODING MODE ----------->\n");

    // Step 1: Open Stego Image
    if (open_decode_files(decInfo) == e_failure)
        return e_failure;

    // Step 2: Verify Magic String
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
        return e_failure;

    // Step 3: Decode Secret File Extension and Create Output File
    int extn_size;
    decode_secret_file_extn_size(decInfo, &extn_size);
    decode_secret_file_extn(decInfo, extn_size);

    // Step 4: Decode Secret File Size and Data
    long secret_size;
    decode_secret_file_size(decInfo, &secret_size);
    decode_secret_file_data(decInfo, secret_size);

    return e_success;
}

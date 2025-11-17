/*

Name : chittooru vinay
Date : 10-11-2025
Project Name : LSB image steganography
Description :
This project implements image steganography using the Least Significant Bit (LSB) 
technique in C, enabling both encoding and decoding of hidden data within BMP images.

In the encoding phase, the algorithm embeds a secret message or file into the cover 
image by replacing the least significant bits of pixel values with the secret data bits, 
ensuring the modified image (stego image) appears visually identical to the original.
The process also encodes essential metadata like the magic string, file extension, and size for
accurate retrieval. 

During the decoding phase, the stego image is processed to extract the hidden 
information by reversing the embedding process, reading LSBs sequentially to reconstruct the original secret file.
This steganography implementation maintains the structural integrity of the BMP format, ensures lossless data recovery, 
and demonstrates secure data concealment through simple yet effective bit-level manipulation.

*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    // Check minimum args
    if(argc < 3)
    {
        printf("  Enter valid Argument:\n");
        printf("  To Encode : ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("  To Decode : ./a.out -d <stego.bmp> [output.txt]\n");
        return e_failure;
    }

    OperationType operation = check_operation_type(argv[1]);
    
    if (operation == e_encode)
    {
        // Encoding requires at least 4 args: ./a.out -e <source.bmp> <secret.txt> [output.bmp]
        if (argc < 4 || argc > 5)
        {
            printf("Invalid number of arguments for encoding.\n");
            printf("Usage: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
            return e_failure;
        }

        printf("\n<----------- ENCODING MODE ----------->\n");
        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Validation successful.\n");
            if (do_encoding(&encInfo) == e_success)
                printf("Encoding Completed Successfully.\n");
            else
                printf("Encoding Failed.\n");
        }
        else
        {
            printf("Validation Failed.\n");
            return e_failure;
        }
    }
    else if(operation == e_decode)
    {
        // Decoding allows 3 or 4 arguments only
        if (argc < 3 || argc > 4)
        {
            printf("Invalid number of arguments for decoding.\n");
            printf("Usage: ./a.out -d <stego.bmp> [output.txt]\n");
            return e_failure;
        }

        printf("\n<----------- DECODING MODE ----------->\n");
        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Validation successful.\n");
            if(do_decoding(&decInfo) == e_success)
            {
                printf("Decoding Completed Successfully.\n");
            }
            else
            {
                printf("Decoding Failed.\n");
            }
        }
        else
        {
            printf("Validation Failed.\n");
            return e_failure;
        }
    }
    else
    {
        printf("Unsupported Operation! Use:\n");
        printf("  -e for Encoding\n");
        printf("  -d for Decoding\n");
        return e_failure;
    }

    return e_success;
}

// Identify operation type
OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

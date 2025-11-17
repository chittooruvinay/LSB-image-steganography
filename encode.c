#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);
    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    // Return image capacity
    return width * height * 3;
}
// Find the size of secret file data
uint get_file_size(FILE *fptr)
{
    
    fseek(fptr, 0, SEEK_END);      // Move to end of file
    uint size = ftell(fptr);       // Get current file position (end = size)
    rewind(fptr);                  // Reset to start
    return size;
}



/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // 1. Validate BMP source file
    if(strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("Invalid: source file must be a .bmp file\n");
        return e_failure;
    }

    // 2. Validate secret file (must have a dot)
    char *dot = strrchr(argv[3], '.');
    if(dot == NULL)
    {
        printf("Invalid: secret file must have an extension (.txt/.c/.sh)\n");
        return e_failure;
    }

    if(strcmp(dot, ".txt") == 0 || strcmp(dot, ".c") == 0 || strcmp(dot, ".sh") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("Invalid: secret file must be a .txt, .c, or .sh file\n");
        return e_failure;
    }

    // 3. Validate optional output BMP file (check if argv[4] exists first)
    if(argv[4] == NULL)
    {
        encInfo->stego_image_fname = "output.bmp";  // default
    }
    else
    {
        if(strstr(argv[4], ".bmp") == NULL)
        {
            printf("Invalid: output file must be a .bmp file\n");
            return e_failure;
        }
        else
        {
            encInfo->stego_image_fname = argv[4];
        }
    }

    return e_success;
}


Status open_files(EncodeInfo *encInfo)
{
    // Open source BMP file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if(encInfo->fptr_src_image == NULL)
    {
        perror("Error opening source BMP file");
        return e_failure;
    }

    // Open secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if(encInfo->fptr_secret == NULL)
    {
        perror("Error opening secret file");
        return e_failure;
    }

    // Open stego image file (output BMP)
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if(encInfo->fptr_stego_image == NULL)
    {
        perror("Error opening output BMP file");
        return e_failure;
    }

    return e_success;
}


Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    
    /* capacity = (magic string size(2*8) + secret extn size(4*8) + secret file extn(4*8) + secret file size(4*8) + secret file data size*8; */
    int capacity = 16 + 32 + 32 + 32 + (encInfo -> size_secret_file * 8);

    if(encInfo->image_capacity > capacity)
    {
        return e_success;   
    }
    else
    {
        return e_failure;

    }
}
        
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Setting pointer to point to 0th position
    rewind(fptr_src_image);
    char header[54];
    // Reading 54 bytes of header from source.bmp
    fread(header, 54, 1, fptr_src_image);
    // Writing the 54 bytes header to destination.bmp
    fwrite(header, 54, 1, fptr_dest_image);
    if(ftell(fptr_src_image) == ftell(fptr_dest_image))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }    

}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] &= 0xFE;                   // clear LSB
        image_buffer[i] |= ((data >> (7 - i)) & 1); // set bit
    }
    return e_success;
}
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] &= 0xFE;
        imageBuffer[i] |= ((size >> (31 - i)) & 1);
    }
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], imageBuffer);
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, imageBuffer);
    fwrite(imageBuffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];
    for(int i=0; i<strlen(file_extn); i++)
    {
        fread(imageBuffer,8 ,1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],imageBuffer);
        fwrite(imageBuffer,8 ,1 ,encInfo->fptr_stego_image);
    }
    return e_success;

}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, imageBuffer);
    fwrite(imageBuffer,32 ,1 ,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    if(!encInfo || !encInfo->fptr_secret || !encInfo->fptr_src_image || !encInfo->fptr_stego_image)
    {
        return e_failure;
    }
   

    rewind(encInfo->fptr_secret);

    int byte_read;
    char imageBuffer[8];

    // Read secret file one byte at a time to avoid large memory use
    while(fread(&byte_read, 1, 1, encInfo->fptr_secret) == 1)
    {
        if(fread(imageBuffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            return e_failure; 
        }
        encode_byte_to_lsb((char)byte_read, imageBuffer);

        if (fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            return e_failure; 
        }
            
    }

    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    printf("Opening files Done\n");
    if(open_files(encInfo) == e_failure)
    {
        return e_failure;
    }
    printf("Checking capacity Done\n");
    if(check_capacity(encInfo) == e_failure)
    {
        return e_failure;
    }
    printf("Copying BMP header Done\n");
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    printf("Encoding magic string Done\n");
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }
    // Get secret file extension (.txt)
    char *extn = strstr(encInfo->secret_fname, ".");
    int extn_size = strlen(extn);

    printf("Encoding file extension size Done\n");
    encode_secret_file_extn_size(extn_size, encInfo);

    printf("Encoding file extension Done\n");
    encode_secret_file_extn(extn, encInfo);

    printf("Encoding secret file size Done\n");
    encode_secret_file_size(encInfo->size_secret_file, encInfo);

    printf("Encoding secret file data Done\n");
    encode_secret_file_data(encInfo);

    printf("Copying remaining image data Done\n");
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    printf("Encoding complete! Stego image saved as %s\n", encInfo->stego_image_fname);
    return e_success;
}
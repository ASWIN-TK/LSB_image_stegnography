#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#define RED "\x1b[31m"
#define SET "\x1b[0m"
#define GREEN "\033[1;32m"

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *temp;
    if(((temp = strstr(argv[2], ".bmp")) != NULL) && (*(temp+4) == '\0'))
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf(RED "\nERROR !!! \nImage File Do not Have Correct Extension\n" SET);
        return e_failure;
    }

    if(argv[3] != NULL)
    {
        if(((temp = strstr(argv[3], ".")) != NULL) && (*(temp+1) != '\0'))
        {
            encInfo->secret_fname = argv[3];
            temp = strstr(argv[3], ".");
            strcpy(encInfo->extn_secret_file, temp);
        }   
        else
        {
            printf(RED "\nERROR!!!\nIncorrect extension for secret file\n" SET);
            return e_failure;
        }
    }
    else
    {
        printf(RED "\n\nERROR!!!\nSecret file not found\n");
        return e_failure;
    }
    if(argv[4] != NULL)
    {
        if(((temp = strstr(argv[4], ".bmp")) != NULL) && (*(temp+4) == '\0'))
        {    
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            if((temp = strstr(argv[4], ".")) != NULL)
            {
                *temp = '\0';
                char *buff = malloc(strlen(argv[4] + 5));
                buff = strcat(argv[4], ".bmp");
                encInfo->stego_image_fname = buff;
                printf("Incorrect extention\n Creating a .bmp file(%s)\n", buff);
                
            }
            else
            {
                char *buff = malloc(strlen(argv[4] + 5));
                buff = strcat(argv[4], ".bmp");
                encInfo->stego_image_fname = buff;
            }
        }
    }
    else
    {
        encInfo->stego_image_fname = "protected.bmp";
    }
    
    return e_success;


}

Status do_encoding(EncodeInfo *encInfo)
{
    if(!open_files(encInfo))
    {
        return e_failure;
    }

    if(!check_capacity(encInfo))
    {
        return e_failure;
    }

    if(!copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image))
    {
        return e_failure;
    }

    encode_magic_size(encInfo->fptr_src_image, encInfo);
    
    if(!encode_magic_string(encInfo->magic, encInfo))
    {
        return e_failure;
    }
    printf(GREEN "\nEncoding Magic String Successfull\n" SET);
    if(!encode_secret_file_extn(encInfo->extn_secret_file, encInfo))
    {
        return e_failure;
    }

    if(!encode_secret_file_size(encInfo->size_secret_file, encInfo))
    {
        return e_failure;
    }

    if(!encode_secret_file_data(encInfo))
    {
        return e_failure;
    }

    printf(GREEN "\nEncodeing Secret Data Successfull\n" SET);
    
    if(!copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image))
    {
        return e_failure;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    unsigned long int p1,p2;
    unsigned char ch;
    p1 = ftell(fptr_src);
    fseek(fptr_src, 0, SEEK_END);
    p2 = ftell(fptr_src);
    fseek(fptr_src, p1, SEEK_SET);

    while(p1 < p2)
    {
        p1++;
        fread(&ch, 1, 1, fptr_src);
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    unsigned char data;
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    int limit = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);
    for(int i = 0; i < limit; i++)
    {
        fread(&data, 1, 1, encInfo->fptr_secret);
        encode_data_to_image(&data, 8, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    unsigned char ch, mask, temp;
    
    for(int i = 0; i < size; i++)
    {
        fread(&ch, 1, 1, fptr_src_image);
        mask = 1 << i;
        temp = ((*data) & mask) >> i;
        ch = ch & (~1);
        ch = ch | temp;
        fwrite(&ch, 1, 1, fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_size(uint file_size, EncodeInfo *encInfo)
{
    unsigned int mask, data;
    unsigned char ch, temp;
    for(int i = 0; i < 32; i++)
    {
        mask = 1 << i;
        data = (file_size & mask) >> i;
        fread(&ch, 1, 1, encInfo->fptr_src_image);
        temp = (char)data;
        ch = ch & (~1);
        ch = ch |temp;
        fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    unsigned int size = strlen(encInfo->extn_secret_file);
    unsigned int mask, data;
    unsigned char ch, temp;
    for(int i = 0; i < 32; i++)
    {
        mask = 1 << i;
        data = (size & mask) >> i;
        fread(&ch, 1, 1, encInfo->fptr_src_image);
        temp = (char)data;
        ch = ch & (~1);
        ch = ch |temp;
        fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
    }
    unsigned char mask1, data1;

    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            mask1 = 1 << j;
            data1 = (file_extn[i] & mask1) >> j;
            fread(&ch, 1, 1, encInfo->fptr_src_image);
            ch = ch & (~1);
            ch = ch | data1;
            fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
        }
    }
    return e_success;
}


Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if(encInfo->fptr_src_image == NULL)
    {
        printf(RED "\nERROR!!! \nSource image is not found in the directry\n"SET);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if(encInfo->fptr_secret == NULL)
    {
        printf(RED "\nERROR!!! \nSecret file is not found in the directry\n"SET);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    printf("\nEnter the magic string: ");
    scanf(" %[^\n]", encInfo->magic);
    encInfo->magic_size = strlen(encInfo->magic);
    int required_size = (4 + encInfo->magic_size + 4 + strlen(encInfo->extn_secret_file) + 4 + encInfo->size_secret_file) * 8;

    if(encInfo->image_capacity < required_size)
    {
        printf(RED "\nERROR!!! \nSource image size is in-sufficient\n" SET);
        return e_failure;
    }
    return e_success;
}

uint get_image_size_for_bmp(FILE *f)
{
    fseek(f, 18, SEEK_SET);
    
    unsigned int height;
    fread(&height, 1, 4, f);
    
    unsigned int width;
    fread(&width, 1, 4, f);
    rewind(f);
    return height*width*3;
}

uint get_file_size(FILE *f)
{
    fseek(f, 0, SEEK_END);
    unsigned int size = ftell(f);
    rewind(f);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buff[54];
    fread(buff, 1, 54, fptr_src_image);
    fwrite(buff, 1, 54, fptr_dest_image);
    return e_success;
}

Status encode_magic_size(FILE *fptr_src_image, EncodeInfo *encInfo)
{
    if(encInfo->magic_size > 10)
    {
        printf(RED "\nERROR\nMagic string exceed the Limit(10 characters)\n" SET);
        return e_failure;
    }

    unsigned int mask, data;
    unsigned char ch, temp;
    for(int i = 0; i < 32; i++)
    {
        mask = 1 << i;
        data = (encInfo->magic_size & mask) >> i;
        fread(&ch, 1, 1, fptr_src_image);
        temp = (char)data;
        ch = ch & (~1);
        ch = ch |temp;
        fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    
    unsigned char mask, data, ch;
    for(int i = 0; i < encInfo->magic_size; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            mask = 1 << j;
            data = (magic_string[i] & mask) >> j;
            fread(&ch, 1, 1, encInfo->fptr_src_image);
            ch = ch & (~1);
            ch = ch | data;
            fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
        }
    }

    return e_success;
}

#include <stdio.h>
#include "types.h"
#include <string.h>
#include "decode.h"
#include <stdlib.h>
#define RED "\x1b[31m"
#define SET "\x1b[0m"
#define GREEN "\033[1;32m"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *temp;
    if(((temp = strstr(argv[2], ".bmp")) != NULL) && ((*(temp + 4)) == '\0'))
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf(RED "\n\nERROR!!!\nNo .bmp extention found");
        return e_failure;
    }

    if(argv[3] != NULL)
    {
        if((temp = strstr(argv[3], ".")) != NULL)
        {
            *temp = '\0';
            strcpy(decInfo->secret_user, argv[3]);
        }
        else
        {
            strcpy(decInfo->secret_user, argv[3]);
        }
    }
    else
    {
        strcpy(decInfo->secret_user, "Output");
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(!open_file_decode(decInfo))
    {
        return e_failure;
    }

    if(!decode_magic_size_data(decInfo->fptr_stego_image, decInfo))
    {
        return e_failure;
    }

    if(!get_ext_decode(decInfo->fptr_stego_image, decInfo))
    {
        return e_failure;
    }

    if(!create_secret_file(decInfo))
    {
        return e_failure;
    }

    if(!get_data_size(decInfo->fptr_stego_image, decInfo))
    {
        return e_failure;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");

    if(!get_data(decInfo->fptr_stego_image, decInfo->fptr_secret, decInfo))
    {
        return e_failure;
    }
    
    return e_success;
}

Status open_file_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if(decInfo->fptr_stego_image == NULL)
    {
        printf(RED "\n\nERROR!!!\nFile is not found in the directry\n" SET);
        return e_failure;
    }
    return e_success;
}

Status decode_magic_size_data(FILE *stego, DecodeInfo *decInfo)
{
    fseek(stego, 54, SEEK_SET);
    printf("Enter the magic string: ");
    char *magic_str = malloc(10);
    scanf(" %[^\n]", magic_str);
    decInfo->magic_size = 0;
    if(strlen(magic_str) > 10)
    {
        printf(RED "\nERROR!!!\nExeeded magic string length\n" SET);
        return e_failure;
    }

    int unsigned data;
    unsigned char ch, buff;
    for(int i = 0; i < 32; i++)
    {
        fread(&ch, 1, 1, stego);
        data = ch & 1;
        decInfo->magic_size = (decInfo->magic_size | (data << i));
    }
    int i;
    for(i = 0; i < decInfo->magic_size; i++)
    {
        buff = 0;
        for(int j = 0; j < 8; j++)
        {
            fread(&ch, 1, 1, stego);
            data = ch & 1;
            buff = (buff | (data << j));
        }
        decInfo->magic_string[i] = buff;
    }
    decInfo->magic_string[i] = '\0';

    if(strcmp(magic_str, decInfo->magic_string) == 0)
    {
        free(magic_str);
        return e_success;
    }
    else
    {
        printf(RED "\nMagic String Does Not Match\n" SET);
        free(magic_str);
        return e_failure;
    }
    
}

Status get_ext_decode(FILE *stego, DecodeInfo *decInfo)
{
    int unsigned size = 0, data;
    unsigned char ch, data1;
    for(int i = 0; i < 32; i++)
    {
        fread(&ch, 1, 1, stego);
        data = ch & 1;
        size = (size | (data << i));
    }

    char unsigned str[10], buff = 0;
    int i;
    for(i = 0; i < size; i++)
    {
        buff = 0;
        for(int j = 0; j < 8; j++)
        {
            fread(&ch, 1, 1, stego);
            data1 = ch & 1;
            buff = (buff | (data1 << j));
        }
        str[i] = buff;
    }

    str[i] = '\0';
    strcpy(decInfo->extn_secret_file, str);
    return e_success;
}

Status create_secret_file(DecodeInfo *decInfo)
{
    char *s = malloc(strlen(decInfo->secret_user)+strlen(decInfo->extn_secret_file));
    strcpy(s,decInfo->secret_user);
    decInfo->secret_fname = strcat(s,decInfo->extn_secret_file);
    return e_success;
}

Status get_data_size(FILE *stego, DecodeInfo *decInfo)
{
    int unsigned size = 0, data;
    unsigned char ch;
    for(int i = 0; i < 32; i++)
    {
        fread(&ch, 1, 1, stego);
        data = ch & 1;
        size = (size | (data << i));
    }
    decInfo->size_secret_file = size;
    return e_success;

}

Status get_data(FILE *stego, FILE *src,DecodeInfo *decInfo)
{
    unsigned char ch, data;
    for(int i = 0; i < decInfo->size_secret_file; i++)
    {
        data = 0;
        for(int j = 0; j < 8; j++)
        {
            fread(&ch, 1, 1, stego);
            ch = ch & 1;
            data = (data | (ch << j));
        }
        fwrite(&data, 1, 1, src);
    }

    return e_success;
}


#ifndef DECODE_H
#define DECODE_H
#include<stdio.h>

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    uint size_secret_file;
    char secret_user[20];

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    int magic_size;
    char magic_string[10];
} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status open_file_decode(DecodeInfo *decInfo);

Status decode_magic_size_data(FILE *stego, DecodeInfo *decInfo);

Status decode_magic(FILE *stego, DecodeInfo *decInfo);

Status get_ext_decode(FILE *stego, DecodeInfo *decInfo);

Status create_secret_file(DecodeInfo *decInfo);

Status get_data_size(FILE *stego, DecodeInfo *decInfo);

Status get_data(FILE *stego, FILE *src, DecodeInfo *decInfo);




#endif

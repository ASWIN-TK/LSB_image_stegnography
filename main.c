#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
#define RED "\x1b[31m"
#define SET "\x1b[0m"
#define GREEN "\033[1;32m"

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf(RED "\nNo Enough Arguments\n\n" SET);
        return e_failure;
    }


    if(check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv, &encInfo))
        {   
            if(do_encoding(&encInfo))
            {
                printf(GREEN "\nSuccessfully encoded the message\n"SET);
                fclose(encInfo.fptr_secret);
                fclose(encInfo.fptr_src_image);
                fclose(encInfo.fptr_stego_image);
                return e_success;
            }
            else
            {
                printf(RED "\nEncoding Failed!!\n" SET);
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        }
    }
    else if(check_operation_type(argv) == e_decode)
    {
        printf(GREEN"You have choose decoding\n"SET);
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv, &decInfo))
        {
            if(do_decoding(&decInfo))
            {
                printf(GREEN "\nSuccessfully decoded the message\n"SET);
                return e_success;
            }
            else
            {
                printf(RED "\nDecoding Failed!!\n" SET);
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        printf(GREEN"\nYou have choose encoding\n"SET);
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        printf(RED "\nERROR ->  Enter Correct argument \n" SET);
        return e_unsupported;
    }
}



#include "u3id.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
//#define _POSIX_C_SOURCE 199309L;

bool check_error(struct Error *error){
    struct Error local_error = *error;
    if(local_error.code != E_SUCCESS){
        printf( "Error code : %d\n", local_error.code);
        printf( "Error message : %s\n", local_error.message);
        return false;
    }
    return true;
}

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        printf(" ");
    }
    puts("");
}

void convert_little_to_big_endian(char * buf, unsigned int len){
    char temp[len];
    int i;

    for (i = 0; i < len; i++) {
        temp[i] = buf[len-1-i];
    }
    memcpy(buf, &temp, len);

}

// maybe use different type than char. try this after
// This function copies memory from src to dest into a specific location in dest defined by bits instead of bytes.
// it will round min_bits_to_copy up to the next nearest byte because the extra bits will be overwritten by the next
// part of the UUUID anyways, and this avoids additional operations
// The src is a normal type and not array
// The dest bit is the location in bits from the beginning of the binary number without worrying about endyness.
// it is not the bit location in memory, it is the bit location of the number
// src and dest are the first byte the start copying. If little endian, this is the greatest byte, if big endian, this is the least byte
// bytes to copy specifies the number of bytes of unshifted data to copy. If the data is shifted, then it will not copy the last tail piece.
void copy_bits_from_source_to_dest(unsigned char *dest, unsigned char *src, size_t bit_shift, unsigned int bytes_to_copy, bool little_endian){
    bool debug = false;
    // TODO: this assumes that other than the first byte, the rest of dest is zeroed out. Make sure we do this.
    if(bit_shift >= 8){
        return;
    }

    // TODO: move dest pointer to dest bit

    // For little endian, if we are shifting the src, then we need to start writing on the previous byte to account
    // for the tail of data after shifting
    unsigned int i;
    unsigned char remainder = 0, bits = 0, mask = 0, testing = 0;

    // For the first byte, we want to leave the bits before the shift untouched, but set the bits in the shift to zero.
    mask = 0xFF << (8-bit_shift);
    *dest &= mask;

    for(i = 0; i < bytes_to_copy; i ++){
        if(debug){
            printf("loop number %i\n", i);
        }


        bits = *src >> bit_shift;

        testing = (remainder | bits);
        *dest |= (remainder | bits);

//        printBits(sizeof(*src), src);
//        printBits(sizeof(bits), &bits);
//        printBits(sizeof(remainder), &remainder);

        remainder = *src << (8-bit_shift);

        if(debug){
            printf("Current src byte: ");
            printBits(1, src);
        }

        if(debug){
            printf("Copying: ");
            printBits(sizeof(testing), &testing);
        }

        if(little_endian) {
            --src;
            --dest;
        } else {
            ++src;
            ++dest;
        }

    }

}



void generate_hash_of_seed(
    char *seed,
    unsigned int seed_length,
    unsigned char *hash_buf
){
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    SHA512_Update(&ctx, seed, seed_length);
    SHA512_Final(hash_buf, &ctx);

//    SHA256_CTX ctx;
//    SHA256_Init(&ctx);
//    SHA256_Update(&ctx, seed, seed_length);
//    SHA256_Final(hash_buf, &ctx);
}



void _generate_u3id(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    uint64_t integer_time_part,
    uint32_t decimal_time_part,
    unsigned char *chaotic_part,
    struct Error *error
){

    bool debug = false;
    unsigned int chaotic_part_length_bits = total_length_bits-timestamp_integer_part_length_bits-timestamp_decimal_part_length_bits;


    if(total_length_bits % 8 != 0){
        // currently only allow whole numbers of bytes due to having to return a number in memory that must be a whole
        // number of bytes. Might be able to find a workaround for this. But probably isn't a useful requirement anyways.
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "total_length_bits must be equal to a whole number of bytes.");
        return;
    }

    if (timestamp_integer_part_length_bits > sizeof(integer_time_part)*8) {
        // Don't allow the user to specify that they want more precision than we have access to.
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "timestamp_integer_part_length_bits must be less than or equal to the length of the integer_time_part.");
        return;
    }

    if (timestamp_decimal_part_length_bits > 30) {
        // Don't allow the user to specify that they want more precision than we have access to.
        // maybe we can allow this later and just automatically add random numbers after this digit
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "timestamp_decimal_part_length_bits must be less than or equal 30 because that is the maximum precision for ns.");
        return;
    }




    //////////
    // the integer time part
    /////////
    // TODO: check for running over the end of the memory
    // this is the little endian implementation. todo: big endian
    // For the integer part, we want to copy starting at the LSB up to timestamp_integer_part_length_bits. For little
    // endian, this is already the order in memory, so we can use memcopy
    int num_bytes_to_copy_for_integer_part = timestamp_integer_part_length_bits/8;

    // whether there is a remainder or not, we want to start putting the decimal part at the next byte.
    int dest_i_for_decimal_part_start = num_bytes_to_copy_for_integer_part + 1;

    if(timestamp_integer_part_length_bits % 8 != 0){
        num_bytes_to_copy_for_integer_part += 1;
        integer_time_part = integer_time_part << (8-(timestamp_integer_part_length_bits % 8));
    }


    memcpy(&uuuid_out[total_length_bits/8-num_bytes_to_copy_for_integer_part], &integer_time_part, num_bytes_to_copy_for_integer_part);

    if(debug){
        printf("num_bytes_to_copy_for_integer_part: %i\n", num_bytes_to_copy_for_integer_part);
        printf("timestamp_integer_part_length_bits modulus 8: %i\n", timestamp_integer_part_length_bits % 8);
        printf("Integral part of time:\n");
        printBits(sizeof(integer_time_part), &integer_time_part);
        printf("UUUID after appending integral amount of time:\n");
        printBits(total_length_bits/8, uuuid_out);
    }




    //////////
    // the fractional time part
    /////////
    // determine the starting point:

    int num_bytes_to_copy_for_decimal_part = timestamp_decimal_part_length_bits/8;
    if(timestamp_decimal_part_length_bits % 8 != 0){
        num_bytes_to_copy_for_decimal_part += 1;
    }

    // The smallest increment we can have is 1x10^(-9).
    // We need 30 bits to capture this full resolution because 2^(-30) = 9.3132257e-10 is the minimum number of bits to make a number smaller than 1x10^(-9)
    // But lets just bit shift it by 31 so that it is shifted all the way to the left of the 64 bit integer and then we can read it directly later as the
    // decimal part.
    // Just remember that we should limit the length of the decimal part to 30 digits because there is no useful information past there.

    unsigned int fractional_part = ((int64_t)decimal_time_part << 32) / 1000000000; // Fractional part.

    unsigned char *fractional_part_src_ptr = (unsigned char *)&fractional_part;
    fractional_part_src_ptr += (int)sizeof(fractional_part) - 1;

    copy_bits_from_source_to_dest(&uuuid_out[total_length_bits/8-dest_i_for_decimal_part_start], fractional_part_src_ptr, timestamp_integer_part_length_bits % 8, num_bytes_to_copy_for_decimal_part, true);

    if(debug){
        printf("The current time is %lld.%.9ld\n", (long long int) integer_time_part, (long int)decimal_time_part);
        printf("num_bytes_to_copy_for_decimal_part: %d\n", num_bytes_to_copy_for_decimal_part);
        printf("fractional_part_src_ptr:\n");
        printBits(sizeof(fractional_part), &fractional_part);
        printf("UUUID after appending decimal amount of time:\n");
        printBits(total_length_bits/8, uuuid_out);
        //return;
    }





    //////////
    // the chaotic part
    /////////
    int dest_i_for_chaotic_part_start = dest_i_for_decimal_part_start + timestamp_decimal_part_length_bits/8;

    int num_bytes_to_copy_for_chaotic_part = chaotic_part_length_bits/8;
    if(num_bytes_to_copy_for_chaotic_part % 8 != 0){
        num_bytes_to_copy_for_chaotic_part += 1;
    }



    if(debug){
        printf("Rand bytes to be added:\n");
        printBits(num_bytes_to_copy_for_chaotic_part, chaotic_part);
        printf("num_bytes_to_copy_for_chaotic_part: %i\n", num_bytes_to_copy_for_chaotic_part);
    }

    unsigned char *chaotic_part_src_ptr = chaotic_part;
    chaotic_part_src_ptr += num_bytes_to_copy_for_chaotic_part-1;

    copy_bits_from_source_to_dest(
        &uuuid_out[total_length_bits/8-dest_i_for_chaotic_part_start],
        chaotic_part_src_ptr,
        (timestamp_integer_part_length_bits+timestamp_decimal_part_length_bits) % 8,
        num_bytes_to_copy_for_chaotic_part,
        true
    );

    if(debug){
        printf("total_length_bits/8-dest_i_for_chaotic_part_start: %i\n", total_length_bits/8-dest_i_for_chaotic_part_start);
        printf("(timestamp_integer_part_length_bits+timestamp_decimal_part_length_bits) mod 8: %i\n", (timestamp_integer_part_length_bits+timestamp_decimal_part_length_bits) % 8);


        printf("UUID after appending chaotic part:\n");

        printBits(total_length_bits/8, uuuid_out);
    }
}



void generate_u3id_supply_chaotic(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    char *chaotic_part_seed,
    unsigned int chaotic_part_seed_length,
    struct Error *error
){

    // get the timestamp info
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    uint64_t integer_time_part = (uint64_t)ts.tv_sec;
    uint32_t decimal_time_part = (uint32_t)ts.tv_nsec;

    // generate chaotic part using hash of provided seed

    unsigned int chaotic_part_length_bits = total_length_bits-timestamp_integer_part_length_bits-timestamp_decimal_part_length_bits;
    if(chaotic_part_length_bits > 512){
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "The chaotic part can be at most 512 bits in length when user is providing seed. (using SHA512 hashing)");
        return;
    }


    unsigned char chaotic_part[64];
    generate_hash_of_seed(chaotic_part_seed, chaotic_part_seed_length, (unsigned char *)&chaotic_part);

    _generate_u3id(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part,
        (unsigned char *)&chaotic_part,
        error
    );
}

void generate_u3id_supply_time(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    uint64_t integer_time_part,
    uint32_t decimal_time_part_ns,
    struct Error *error
){

    if(decimal_time_part_ns > 999999999){
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "When providing a time component, the decimal time part must be specified in ns and be at most 999,999,999");
    }

    unsigned int chaotic_part_length_bits = total_length_bits-timestamp_integer_part_length_bits-timestamp_decimal_part_length_bits;
    unsigned char *chaotic_part;
    chaotic_part = (unsigned char *)calloc(chaotic_part_length_bits/8+1, sizeof(char));

    if(!RAND_bytes(chaotic_part, chaotic_part_length_bits/8+1)){
        //failed to generate chaotic part
        error->code = E_RAND_ERROR;
        strcpy( error->message, "Failed to generate random bytes.");
        return;
    }

    _generate_u3id(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part_ns,
        chaotic_part,
        error
    );

}

void generate_u3id_supply_all(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    uint64_t integer_time_part,
    uint32_t decimal_time_part_ns,
    char *chaotic_part_seed,
    unsigned int chaotic_part_seed_length,
    struct Error *error
){

    if(decimal_time_part_ns > 999999999){
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "When providing a time component, the decimal time part must be specified in ns and be at most 999,999,999");
    }

    unsigned int chaotic_part_length_bits = total_length_bits-timestamp_integer_part_length_bits-timestamp_decimal_part_length_bits;
    if(chaotic_part_length_bits > 512){
        error->code = E_VALUE_ERROR;
        strcpy( error->message, "The chaotic part can be at most 512 bits in length when user is providing seed. (using SHA512 hashing)");
        return;
    }

    unsigned char chaotic_part[64];
    generate_hash_of_seed(chaotic_part_seed, chaotic_part_seed_length, (unsigned char *)&chaotic_part);

    _generate_u3id(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part_ns,
        chaotic_part,
        error
    );

}


void generate_u3id_std(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    struct Error *error
){
    // get the timestamp info
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    uint64_t integer_time_part = ts.tv_sec;
    uint32_t decimal_time_part = (uint32_t)ts.tv_nsec;

    unsigned int chaotic_part_length_bits = total_length_bits-timestamp_integer_part_length_bits-timestamp_decimal_part_length_bits;
    unsigned char *chaotic_part;
    chaotic_part = (unsigned char *)calloc(chaotic_part_length_bits/8+1, sizeof(char));

    if(!RAND_bytes(chaotic_part, chaotic_part_length_bits/8+1)){
        //failed to generate chaotic part
        error->code = E_RAND_ERROR;
        strcpy( error->message, "Failed to generate random bytes.");
        return;
    }

    _generate_u3id(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part,
        chaotic_part,
        error
    );

    free(chaotic_part);

}





//////////////////////////
///// TESTS
/////////////////////////
void test_std(){
    // var to hold any exceptions
    struct Error error = {E_SUCCESS, ""};

    unsigned int timestamp_integer_part_length_bits = 32; // 32; up to 136 years from 1970
    unsigned int timestamp_decimal_part_length_bits = 10; // 10; 1024, or about 1 ms precision
    unsigned int total_length_bits = 128;


    unsigned char *uuuid_out;
    uuuid_out = (unsigned char *)calloc((total_length_bits)/8, sizeof(char));


    generate_u3id_std(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        &error
    );

    check_error(&error);

    printBits(total_length_bits/8+1, uuuid_out);

    convert_little_to_big_endian(uuuid_out, 16);
    printBits(total_length_bits/8+1, uuuid_out);
}


void test_supply_time(){
    // var to hold any exceptions
    struct Error error = {E_SUCCESS, ""};

    unsigned int timestamp_integer_part_length_bits = 32; // 32; up to 136 years from 1970
    unsigned int timestamp_decimal_part_length_bits = 30; // 10; 1024, or about 1 ms precision
    unsigned int total_length_bits = 128;

    unsigned char *uuuid_out;
    uuuid_out = (unsigned char *)calloc((total_length_bits)/8, sizeof(char));

    uint64_t integer_time_part = 100;
    uint32_t decimal_time_part_ns = 999999999;

    generate_u3id_supply_time(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part_ns,
        &error
    );

    check_error(&error);

    printBits(total_length_bits/8+1, uuuid_out);
}

void test_supply_all(){
    // var to hold any exceptions
    struct Error error = {E_SUCCESS, ""};

    unsigned int timestamp_integer_part_length_bits = 32; // 32; up to 136 years from 1970
    unsigned int timestamp_decimal_part_length_bits = 30; // 10; 1024, or about 1 ms precision
    unsigned int total_length_bits = 128;


    unsigned char *uuuid_out;
    uuuid_out = (unsigned char *)calloc((total_length_bits)/8, sizeof(char));

    uint64_t integer_time_part = 100;
    uint32_t decimal_time_part_ns = 599999999;

    char *chaotic_part_seed;
    char *str = "this is a test";
    int chaotic_part_seed_length = strlen(str);

    chaotic_part_seed = (char *)malloc(chaotic_part_seed_length+1);
    strcpy(chaotic_part_seed, str);


    generate_u3id_supply_all(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        integer_time_part,
        decimal_time_part_ns,
        chaotic_part_seed,
        chaotic_part_seed_length,
        &error
    );

    check_error(&error);

    printBits(total_length_bits/8+1, uuuid_out);
}

void test_supply_chaotic(){
    // var to hold any exceptions
    struct Error error = {E_SUCCESS, ""};

    unsigned int timestamp_integer_part_length_bits = 0; // 32; up to 136 years from 1970
    unsigned int timestamp_decimal_part_length_bits = 0; // 10; 1024, or about 1 ms precision
    unsigned int total_length_bits = 128;


    unsigned char *uuuid_out;
    uuuid_out = (unsigned char *)calloc((total_length_bits)/8, sizeof(char));


    char *chaotic_part_seed;
    char *str = "this is a test";
    int chaotic_part_seed_length = strlen(str);

    chaotic_part_seed = (char *)malloc(chaotic_part_seed_length+1);
    strcpy(chaotic_part_seed,str);

    generate_u3id_supply_chaotic(
        uuuid_out,
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        chaotic_part_seed,
        chaotic_part_seed_length,
        &error
    );

    check_error(&error);

//    convert_little_to_big_endian(uuuid_out, total_length_bits/8);

    printBits(total_length_bits/8, uuuid_out);
}

void test_pg(void){
    struct Error error = {E_SUCCESS, ""};

    unsigned int timestamp_integer_part_length_bits = 32; // 32; up to 136 years from 1970
    unsigned int timestamp_decimal_part_length_bits = 10; // 10; 1024, or about 1 ms precision
    unsigned int total_length_bits = 128;

    unsigned char uuuid_out[16];
//    unsigned char *uuuid_out;
//    uuuid_out = (unsigned char *)calloc((total_length_bits)/8, sizeof(char));

    generate_u3id_std(
        (unsigned char*)(&uuuid_out),
        timestamp_integer_part_length_bits,
        timestamp_decimal_part_length_bits,
        total_length_bits,
        &error
    );


    printBits(16, &uuuid_out);
    convert_little_to_big_endian((unsigned char*)(&uuuid_out), 16);
    printBits(16, &uuuid_out);
}

//int main(void)
//{
////    test_std();
////    test_pg();
//    test_supply_chaotic();
////    test_supply_time();
//    test_supply_all();
//}
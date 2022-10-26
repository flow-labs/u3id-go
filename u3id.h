#ifndef U3ID_H_
#define U3ID_H_

#include <stdint.h>
#include <time.h>

enum _error_code
{
    E_SUCCESS,
    E_VALUE_ERROR,
    E_RAND_ERROR,
};

struct Error {
    enum _error_code code;
    char message[500];
};
void convert_little_to_big_endian(char * buf, unsigned int len);

void generate_u3id_std(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    struct Error *error
);

void generate_u3id_supply_chaotic(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    char *chaotic_part_seed,
    unsigned int chaotic_part_seed_length,
    struct Error *error
);

void generate_u3id_supply_time(
    unsigned char *uuuid_out,
    unsigned int timestamp_integer_part_length_bits,
    unsigned int timestamp_decimal_part_length_bits,
    unsigned int total_length_bits,
    uint64_t integer_time_part,
    uint32_t decimal_time_part_ns,
    struct Error *error
);

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
);

#endif
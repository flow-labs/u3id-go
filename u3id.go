package u3id

//package main

// #cgo LDFLAGS: -lssl -lcrypto
// #cgo CFLAGS: -I /usr/include/openssl
// #include <stdlib.h>
// #include "u3id.h"
import "C"
import (
	"errors"
	"unsafe"
)

func get_golang_error(c_error C.struct_Error) error {
	if c_error.code != C.E_SUCCESS {
		message := (*C.char)(unsafe.Pointer(&c_error.message[0]))
		return errors.New(C.GoStringN(message, 500))
	} else {
		return nil
	}
}

func U3id_s(
	timestamp_integer_part_length_bits int,
	timestamp_decimal_part_length_bits int,
	total_length_bits int,
) ([]byte, error) {
	c_error := C.struct_Error{}
	c_error.code = 0

	c_timestamp_integer_part_length_bits := C.uint(timestamp_integer_part_length_bits)
	c_timestamp_decimal_part_length_bits := C.uint(timestamp_decimal_part_length_bits)
	c_total_length_bits := C.uint(total_length_bits)

	uuuid_out := C.malloc((C.ulong)(c_total_length_bits / 8))
	defer C.free(unsafe.Pointer(uuuid_out))

	C.generate_u3id_std(
		(*C.uchar)(uuuid_out),
		c_timestamp_integer_part_length_bits,
		c_timestamp_decimal_part_length_bits,
		c_total_length_bits,
		&c_error,
	)

	C.convert_little_to_big_endian((*C.char)(uuuid_out), (C.uint)(c_total_length_bits/8))
	go_u3id := C.GoBytes(uuuid_out, (C.int)(c_total_length_bits/8))
	return go_u3id, get_golang_error(c_error)
}

func U3id_c(
	timestamp_integer_part_length_bits int,
	timestamp_decimal_part_length_bits int,
	total_length_bits int,
	chaotic_part_seed string,
) ([]byte, error) {
	c_error := C.struct_Error{}
	c_error.code = 0

	c_timestamp_integer_part_length_bits := C.uint(timestamp_integer_part_length_bits)
	c_timestamp_decimal_part_length_bits := C.uint(timestamp_decimal_part_length_bits)
	c_total_length_bits := C.uint(total_length_bits)

	uuuid_out := C.malloc((C.ulong)(c_total_length_bits / 8))
	defer C.free(unsafe.Pointer(uuuid_out))

	c_chaotic_part_seed := C.CString(chaotic_part_seed)
	defer C.free(unsafe.Pointer(c_chaotic_part_seed))

	chaotic_part_len := len([]rune(chaotic_part_seed))

	C.generate_u3id_supply_chaotic(
		(*C.uchar)(uuuid_out),
		c_timestamp_integer_part_length_bits,
		c_timestamp_decimal_part_length_bits,
		c_total_length_bits,
		c_chaotic_part_seed,
		(C.uint)(chaotic_part_len),
		&c_error,
	)

	C.convert_little_to_big_endian((*C.char)(uuuid_out), (C.uint)(c_total_length_bits/8))
	go_u3id := C.GoBytes(uuuid_out, (C.int)(c_total_length_bits/8))
	return go_u3id, get_golang_error(c_error)
}

func U3id_t(
	timestamp_integer_part_length_bits int,
	timestamp_decimal_part_length_bits int,
	total_length_bits int,
	integer_time_part int64,
	decimal_time_part_ns int32,
) ([]byte, error) {
	c_error := C.struct_Error{}
	c_error.code = 0

	c_timestamp_integer_part_length_bits := C.uint(timestamp_integer_part_length_bits)
	c_timestamp_decimal_part_length_bits := C.uint(timestamp_decimal_part_length_bits)
	c_total_length_bits := C.uint(total_length_bits)

	uuuid_out := C.malloc((C.ulong)(c_total_length_bits / 8))
	defer C.free(unsafe.Pointer(uuuid_out))

	C.generate_u3id_supply_time(
		(*C.uchar)(uuuid_out),
		c_timestamp_integer_part_length_bits,
		c_timestamp_decimal_part_length_bits,
		c_total_length_bits,
		(C.ulong)(integer_time_part),
		(C.uint)(decimal_time_part_ns),
		&c_error,
	)

	C.convert_little_to_big_endian((*C.char)(uuuid_out), (C.uint)(c_total_length_bits/8))
	go_u3id := C.GoBytes(uuuid_out, (C.int)(c_total_length_bits/8))
	return go_u3id, get_golang_error(c_error)
}

func U3id_a(
	timestamp_integer_part_length_bits int,
	timestamp_decimal_part_length_bits int,
	total_length_bits int,
	integer_time_part int64,
	decimal_time_part_ns int32,
	chaotic_part_seed string,
) ([]byte, error) {
	c_error := C.struct_Error{}
	c_error.code = 0

	c_timestamp_integer_part_length_bits := C.uint(timestamp_integer_part_length_bits)
	c_timestamp_decimal_part_length_bits := C.uint(timestamp_decimal_part_length_bits)
	c_total_length_bits := C.uint(total_length_bits)

	uuuid_out := C.malloc((C.ulong)(c_total_length_bits / 8))
	defer C.free(unsafe.Pointer(uuuid_out))

	c_chaotic_part_seed := C.CString(chaotic_part_seed)
	defer C.free(unsafe.Pointer(c_chaotic_part_seed))

	chaotic_part_len := len([]rune(chaotic_part_seed))

	C.generate_u3id_supply_all(
		(*C.uchar)(uuuid_out),
		c_timestamp_integer_part_length_bits,
		c_timestamp_decimal_part_length_bits,
		c_total_length_bits,
		(C.ulong)(integer_time_part),
		(C.uint)(decimal_time_part_ns),
		c_chaotic_part_seed,
		(C.uint)(chaotic_part_len),
		&c_error,
	)

	C.convert_little_to_big_endian((*C.char)(uuuid_out), (C.uint)(c_total_length_bits/8))
	go_u3id := C.GoBytes(uuuid_out, (C.int)(c_total_length_bits/8))
	return go_u3id, get_golang_error(c_error)
}

// TODO: remove duplicate
//func main() {
//
//	//u3id, err := u3id_s(32, 10, 128)
//	//u3id, err := u3id_c(0, 0, 128, "this is a test")
//	//u3id, err := u3id_t(
//	//	32,
//	//	30,
//	//	128,
//	//	100,
//	//	999999999,
//	//)
//	u3id, err := u3id_a(
//		32,
//		30,
//		128,
//		100,
//		999999999,
//		"this is a test",
//	)
//
//	if err != nil {
//		log.Fatal(err)
//	}
//
//	fmt.Printf("%x", u3id)
//
//	//b := C.GoBytes(ptr, size)
//	//fmt.Println(string(b))
//
//	//name := C.CString("Gopher")
//	//defer C.free(unsafe.Pointer(name))
//	//
//	//year := C.int(2018)
//	//
//	//ptr := C.malloc(128)
//	//defer C.free(unsafe.Pointer(ptr))
//	//
//	//size := C.greet(name, year, (*C.char)(ptr))
//	//
//	//b := C.GoBytes(ptr, size)
//	//fmt.Println(string(b))
//}

// todo: doc
// This golang package will require openssl being installed.
// need to add this install instruction in doc

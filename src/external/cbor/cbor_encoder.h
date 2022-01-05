#pragma once

// https://github.com/vladimirgamalyan/cbor

#include <cstdint>

class cbor_encoder {
public:
	void write_null() { put_byte(0xf6); }

	void write_undefined() { put_byte(0xf7); }

	void write_bool(bool value) { put_byte(value ? 0xf5 : 0xf4); }

	void write_break() { put_byte(0xff); }

	void write_uint(uint64_t value) { write_type_and_value(0, value); }

	void write_int(int64_t value) {
		if (value < 0)
			write_type_and_value(1, -(value + 1));
		else
			write_type_and_value(0, value);
	}

	void write_bytes_header(uint64_t size) { write_type_and_value(2, size); }

	void write_string_header(uint64_t size) { write_type_and_value(3, size); }

	void write_array(uint64_t size) { write_type_and_value(4, size); }

	void write_indefinite_array() { put_byte(0x9f); }

	void write_map(uint64_t size) { write_type_and_value(5, size); }

	void write_indefinite_map() { put_byte(0xbf); }

	void write_tag(uint64_t tag) { write_type_and_value(6, tag); }

protected:
	virtual void put_byte(uint8_t b) = 0;

	void write_type_and_value(uint8_t major_type, uint64_t value) {
		major_type <<= 5u;
		if (value < 24) {
			put_byte((uint8_t)(major_type | value));
		}
		else if (value < 256) {
			put_byte((uint8_t)(major_type | 24u));
			put_byte((uint8_t)value);
		}
		else if (value < 65536) {
			put_byte((uint8_t)(major_type | 25u));
			put_byte((uint8_t)(value >> 8u));
			put_byte((uint8_t)value);
		}
		else if (value < 4294967296ULL) {
			put_byte((uint8_t)(major_type | 26u));
			put_byte((uint8_t)(value >> 24u));
			put_byte((uint8_t)(value >> 16u));
			put_byte((uint8_t)(value >> 8u));
			put_byte((uint8_t)value);
		}
		else {
			put_byte((uint8_t)(major_type | 27u));
			put_byte((uint8_t)(value >> 56u));
			put_byte((uint8_t)(value >> 48u));
			put_byte((uint8_t)(value >> 40u));
			put_byte((uint8_t)(value >> 32u));
			put_byte((uint8_t)(value >> 24u));
			put_byte((uint8_t)(value >> 16u));
			put_byte((uint8_t)(value >> 8u));
			put_byte((uint8_t)(value));
		}
	}
};

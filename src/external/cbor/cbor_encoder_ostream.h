#pragma once

#include <ostream>
#include <string>
#include <vector>
#include "cbor_encoder.h"

// https://github.com/vladimirgamalyan/cbor

class cbor_encoder_ostream : public cbor_encoder {
public:
	explicit cbor_encoder_ostream(std::ostream &os) : os(os) {}

	void write_bytes(const uint8_t *data, size_t size) {
		write_bytes_header(size);
		os.write((const char *)data, size);
	}

	void write_bytes(const std::vector<uint8_t> &data) {
		write_bytes(&data[0], data.size());
	}

	void write_string(const std::string &s) {
		write_string_header(s.size());
		os.write(s.c_str(), s.size());
	}

private:
	std::ostream &os;
	void put_byte(uint8_t b) override { os.put((char)b); }
};

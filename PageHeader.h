#include <iostream>
#include <fstream>
#include "BitMap.h"

template<typename U>
class PageHeader {
	template<typename R> friend class FixedLengthFile;

public:
	std::streampos next_header_pos{};
	size_t available_space{};
	BitMap bitmap{};
public:
	PageHeader(const std::streampos& next_header_pos, const unsigned& num_records) : next_header_pos(next_header_pos), available_space(0) {
		bitmap.inicializarNuevoMapa(num_records);
		available_space = num_records * sizeof(U);
	}

	PageHeader() : next_header_pos(0), available_space(0) {}
	friend std::ofstream& operator<<(std::ofstream& out, const PageHeader<U>& c) {
		out.write(reinterpret_cast<const char*>(&c.next_header_pos), sizeof(c.next_header_pos));
		out.write(reinterpret_cast<const char*>(&c.available_space), sizeof(c.available_space));
		out << c.bitmap;
		return out;
	}

	friend std::ifstream& operator>>(std::ifstream& in, PageHeader<U>& c) {
		in.read(reinterpret_cast<char*>(&c.next_header_pos), sizeof(c.next_header_pos));
		in.read(reinterpret_cast<char*>(&c.available_space), sizeof(c.available_space));
		in >> c.bitmap;
		return in;
	}

	friend std::ostream& operator<<(std::ostream& os, const PageHeader<U>& c) {
		return os << "Espacio Disponible: " << c.available_space << "\nSiguiente Cabezera: " << c.next_header_pos
			<< "\nMapa de Bits: " << c.bitmap << '\n';
	}

	static auto getFixedSize() {
		return sizeof(next_header_pos) + sizeof(available_space);
	}
};


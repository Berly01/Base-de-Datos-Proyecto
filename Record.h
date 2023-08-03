#pragma once
#ifndef _RECORD_H_
#define _RECORD_H_
#include <iostream>
#include <fstream>

template<typename U>
class Record {
	template<typename R> friend class FixedLengthFile;
private:
	unsigned id{ 1 };
	U dato;
public:
	Record (const U& dato, const unsigned& id) : dato(dato), id(id) {}

	Record() {}

	friend std::ofstream& operator<<(std::ofstream& out, const Record<U>& e) {
		out.write(reinterpret_cast<const char*>(&e), sizeof(e));
		return out;
	}

	friend std::ifstream& operator>>(std::ifstream& in, Record<U>& e) {
		in.read(reinterpret_cast<char*>(&e), sizeof(e));
		return in;
	}

	friend std::ostream& operator<<(std::ostream& os, const Record<U>& r) {
		os << r.dato;
		return os;
	}
};

#endif // !_RECORD_H_
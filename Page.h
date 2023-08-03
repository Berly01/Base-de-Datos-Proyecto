#pragma once
#include <iostream>
#include <vector>
#include <fstream>

#include "FixedLengthFile.h"


template<typename U>
class Page {
	template<typename R> friend class BufferPoolManager;
public:
	bool dirty_bit{}, pinnig{};
	int page_id{}, pin_count{}, count{};
	std::streampos page_header_pos{};
	std::vector<Record<U>> vec_records{};
	PageHeader<U> header_page;
public:
	Page(const int& page_id, FixedLengthFile<U>* file) : page_id(page_id) {
		dirty_bit = false;
		pinnig = true;
		pin_count = 1;
		count = 0;
		header_page = std::get<1>(file->get_header_page(page_id));

		std::ifstream in_file(file->getFilePath(), std::ios::in | std::ios::binary);

		in_file.seekg(file->get_records_pos(page_id));

		const auto limit_pos{ header_page.next_header_pos };

		Record<U> empty_record;

		while (in_file.tellg() < limit_pos && !in_file.eof()) {
			in_file >> empty_record;
			vec_records.push_back(empty_record);
		}

		in_file.close();
	}

	Page() : page_id(-1) {
		dirty_bit, pinnig = false;
		pin_count, count = -1;
	}

	~Page() {}

	void reset() {
		dirty_bit, pinnig = false;
		page_id, pin_count, count = -1;
		if (!vec_records.empty()) { vec_records.clear(); }
	}

	friend std::ostream& operator<<(std::ostream& os, const Page& page) {
		return os << page.page_id;
	}
};


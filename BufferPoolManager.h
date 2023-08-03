#pragma once
#include "Page.h"

template<typename T>
class BufferPoolManager {
	template<typename R> friend class MRU;
public:
	FixedLengthFile<T>* file;
	const unsigned NUM_FRAMES{};
	std::unordered_map<char, Page<T>> map_pag{};

public:
	BufferPoolManager(FixedLengthFile<T>* file, const unsigned& NUM_FRAMES) : file(file), NUM_FRAMES(NUM_FRAMES) {
		for (char i = 65; i < 65 + NUM_FRAMES; i++) {
			map_pag.insert(std::make_pair(i, Page<T>()));
		}
	}

	auto find_free_space() const {
		for (const auto& i : map_pag) {
			if (i.second.page_id == -1)
				return i.first;
		}
		return '0';
	}

	char exiting_page(const int& page_id) const {
		for (const auto& i : map_pag) {
			if (i.second.page_id == page_id) { return i.first; }
		}
		return '0';
	}

	bool free_frame_id() const {
		for (const auto& i : map_pag) {
			if (i.second.page_id == -1) { return true; }
		}
		return false;
	}

	void pin_page(const char& frame_id) {
		map_pag[frame_id].pin_count++;
	}

	int get_page_id(const char& frame_id) {
		for (auto& i : map_pag) {
			if (i.first == frame_id) {
				return i.second.page_id;
			}
		}
		return -1;
	}

	void read_page(const char& frame_id) {
		std::cout << map_pag[frame_id].vec_records.size();
		for (const auto& i : map_pag[frame_id].vec_records) {
			std::cout << i << '\n';
		}
	}

	auto fetch_page(const int& page_id) const {
		for (const auto& i : map_pag) {
			if (i.second.page_id == page_id) {
				return i.second;
			}
		}
		throw std::invalid_argument("La pagina indicada no existe!\n");
	}

	void new_page(const int& page_id, const char& frame_id = '0') {
		map_pag[frame_id].reset();
		map_pag[frame_id] = Page<T>(page_id, file);
	}

	void un_pin_page(const int& page_id, const bool& is_dirty) {

		if (map_pag[page_id].pin_count != 0) {
			map_pag[page_id].pin_count--;
		}

		if (is_dirty) {
			for (auto& i : map_pag) {
				if (i.second.page_id == page_id) {
					i.second.dirty_bit = true;
					return;
				}
			}
		}
	}

	void flush_page(const int& page_id) {
		for (const auto& i : map_pag) {
			if (i.second.page_id == page_id) {
				file->writePage(i.second.page_id, i.second.vec_records);
				return;
			}
		}
		return;
	}

	void flush_all_pages() {
		for (const auto& i : map_pag) {
			if (i.second.page_id != -1) {
				file->writePage(i.second.page_id, i.second.vec_records);
			}
		}
		return;
	}

	void delete_page(const int& page_id) {
		map_pag[page_id].reset();
	}

	friend std::ostream& operator<<(std::ostream& os, const BufferPoolManager& b) {
		for (const auto& i : b.map_pag) {
			os << i.first << " " << i.second << '\n';
		}
		return os;
	}
};


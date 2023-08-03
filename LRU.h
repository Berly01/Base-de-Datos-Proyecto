#pragma once
#include <queue>
#include "BufferPoolManager.h"

template<typename T>
class LRU {

private:
	BufferPoolManager<T>* buffer_pool;
	std::queue<char> last_use_frame{};
public:

	LRU(BufferPoolManager<T>* buffer_pool) : buffer_pool(buffer_pool) {}

	void update_pin_count(const char& exception_frame) {
		for (auto& i : buffer_pool->map_pag) {
			if (i.first == exception_frame) {
				continue;
			}
			else if (i.second.page_id != -1 && i.second.pin_count != 0) {
				i.second.pin_count--;

				if (i.second.pin_count == 0) {
					last_use_frame.push(i.first);
				}
			}
		}
	}

	void update_last_use_frame(const char& frame_id) {
		std::queue<char> aux_queue{};
		char aux_char{};
		while (!last_use_frame.empty()) {

			aux_char = last_use_frame.front();
			last_use_frame.pop();

			if (frame_id == aux_char) {
				continue;
			}
			aux_queue.push(aux_char);
		}
		last_use_frame = aux_queue;
	}

	void insert_page(const unsigned& page_id) {
		char frame_id{ buffer_pool->exiting_page(page_id) };

		if (frame_id != '0') {
			buffer_pool->pin_page(frame_id);
			update_last_use_frame(frame_id);
		}
		else {
			frame_id = buffer_pool->find_free_space();

			if (frame_id != '0') {
				buffer_pool->new_page(page_id, frame_id);
			}
			else {
				auto top = last_use_frame.front();
				last_use_frame.pop();
				buffer_pool->delete_page(top);
				buffer_pool->new_page(page_id, top);
				frame_id = top;
			}
		}
		update_pin_count(frame_id);
	}

	void read_page(const char& frame_id) {
		buffer_pool->read_page(frame_id);
	}

	void write_page(const char& frame_id, const T& record, const unsigned& record_pos = 0) {

		/*
		if (record_pos >= buffer_pool->file->records_per_pages
			|| buffer_pool->map_pag[frame_id].vec_records.size() >= record_pos) {
			throw std::out_of_range("Posicion fuera del rango de los registros\n");
		}
		*/

		Record<T> new_records(record, 100);

		if (record_pos == 0) {
			buffer_pool->map_pag[frame_id].vec_records.push_back(new_records);
		}
		else {
			buffer_pool->map_pag[frame_id].vec_records[record_pos] = new_records;
		}

		buffer_pool->un_pin_page(buffer_pool->get_page_id(frame_id), true);

		buffer_pool->flush_page(buffer_pool->get_page_id(frame_id));

		return;
	}

	void info_frames() const {
		for (const auto& i : buffer_pool->map_pag) {
			std::cout << i.first << " " << "pin_count: " << i.second.pin_count
				<< "  dirty_bit: " << i.second.dirty_bit << '\n';
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const LRU<T>& lru) {
		return os << *lru.buffer_pool;
	}
};

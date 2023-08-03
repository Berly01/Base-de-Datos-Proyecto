#include <iostream>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "PageHeader.h"
#include "Record.h"

template<typename T>
class FixedLengthFile {
	template<typename R> friend class BufferPoolManager;

	class MainHeader {
		template<typename R> friend class FixedLengthFile;
	private:
		unsigned total_pages{};
		unsigned records_per_pages{};
	public:
		MainHeader(const unsigned& num_total_pags, const unsigned& campos_por_pag) : total_pages(num_total_pags), records_per_pages(campos_por_pag) {}
		MainHeader() : total_pages(0), records_per_pages(0) {}
		friend std::ofstream& operator<<(std::ofstream& out, const MainHeader& c) {
			out.write(reinterpret_cast<const char*>(&c), sizeof(c));
			return out;
		}
		friend std::ifstream& operator>>(std::ifstream& in, MainHeader& c) {
			in.read(reinterpret_cast<char*>(&c), sizeof(c));
			return in;
		}
	};

public:
	using PointerPos_PageHeader = std::tuple<std::streampos, PageHeader<T>>;
	unsigned total_pages{};
	unsigned records_per_pages{};
	unsigned current_page{};
	size_t PAGE_SIZE{};
	size_t PAGE_HEADER_SIZE{};
	const std::string FILE_PATH{};
	std::unordered_map<unsigned, PointerPos_PageHeader> map_header_page{};

public:
	void setCabezalPagina(const unsigned& page) {
		if (page >= total_pages)
			throw std::invalid_argument("El archivo no contiene el numero de paginas ingresado\n");

		std::ofstream output_file(FILE_PATH, std::ios::in, std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		output_file.seekp(std::get<0>(map_header_page[page]));
		output_file << std::get<1>(map_header_page[page]);
		output_file.close();
	}

	auto get_header_page(const unsigned& num_page) {
		return map_header_page[num_page];
	}

	auto getPosCampo(const unsigned& page, const unsigned& pos) {
		if (pos >= records_per_pages)
			throw std::invalid_argument("La indicada sale del rango de la cantidad maxima de campos\n");
		const auto record_pos_bytes{ static_cast<size_t>(std::get<0>(map_header_page[page])) + PAGE_HEADER_SIZE + sizeof(Record<T>) * (pos - 1) };
		return record_pos_bytes;
	}

public:
	FixedLengthFile(const std::string_view& FILE_PATH, const unsigned& total_pages, const unsigned& records_per_pages)
		: FILE_PATH(FILE_PATH) {

		this->total_pages = total_pages;
		this->records_per_pages = records_per_pages;
		this->current_page = 0;

		PAGE_HEADER_SIZE = PageHeader<T>::getFixedSize() + sizeof(unsigned) + sizeof(bool) * records_per_pages;
		PAGE_SIZE = sizeof(Record<T>) * records_per_pages + PAGE_HEADER_SIZE;

		std::ofstream output_file(this->FILE_PATH, std::ios::out | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		auto main_header = MainHeader(total_pages, records_per_pages);
		output_file << main_header;

		auto header_position{ static_cast<size_t>(output_file.tellp()) };

		Record<T> empty_record;

		for (unsigned i = 0; i < total_pages; i++) {
			PageHeader<T> page_header(header_position + (i + 1) * PAGE_SIZE, records_per_pages);
			map_header_page.insert(std::make_pair(i, std::make_tuple(header_position + i * PAGE_SIZE, page_header)));

			output_file << page_header;

			for (unsigned e = 1; e <= records_per_pages; e++) {
				empty_record.id = e + records_per_pages * i;
				output_file << empty_record;
			}
		}
		output_file.close();
	}

	FixedLengthFile(const std::string_view& FILE_PATH) : FILE_PATH(FILE_PATH) {
		std::filesystem::path file_path(FILE_PATH);
		if (!std::filesystem::exists(file_path)) {
			throw std::invalid_argument("La direccion del archivo ingresado no es valida!\n");
		}

		current_page = 0;

		std::ifstream input_file(this->FILE_PATH, std::ios::in, std::ios::binary);

		if (!input_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		MainHeader main_header;
		input_file >> main_header;

		this->total_pages = main_header.total_pages;
		this->records_per_pages = main_header.records_per_pages;

		PAGE_HEADER_SIZE = PageHeader<T>::getFixedSize() + sizeof(unsigned) + sizeof(bool) * records_per_pages;
		PAGE_SIZE = PAGE_HEADER_SIZE + sizeof(Record<T>) * records_per_pages;

		auto header_position{ sizeof(MainHeader) };

		for (unsigned i = 0; i < total_pages; i++) {

			PageHeader<T> page_header;
			input_file >> page_header;

			map_header_page.insert(std::make_pair(i, std::make_tuple(header_position + i * PAGE_SIZE, page_header)));

			input_file.seekg(page_header.next_header_pos);
		}

		input_file.close();
	}

	~FixedLengthFile() {}

	auto getPageSize() const {
		return PAGE_SIZE;
	}

	auto getActualPage() const {
		return current_page;
	}

	void setActualPage(const unsigned& current_page) {
		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");
		this->current_page = current_page;
	}

	auto getTotalPages() const {
		return total_pages;
	}

	auto getFilePath() const {
		return FILE_PATH;
	}

	auto get_page_size() const {
		return PAGE_SIZE;
	}

	auto get_page_header_size() const {
		return PAGE_HEADER_SIZE;
	}

	auto get_records_pos(const unsigned& num_page) {
		return static_cast<size_t>(std::get<0>(map_header_page[num_page])) + PAGE_HEADER_SIZE;
	}

	void readPage(const unsigned& current_page) {

		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indicada!\n");

		std::ifstream input_file(FILE_PATH, std::ios::in | std::ios::binary);
		if (!input_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'\n";
			exit(EXIT_FAILURE);
		}

		input_file.seekg(static_cast<size_t>(std::get<0>(map_header_page[current_page])) + PAGE_HEADER_SIZE);

		auto pos_limite{ std::get<1>(map_header_page[current_page]).next_header_pos };

		Record<T> campo;

		while (input_file.tellg() < pos_limite && !input_file.eof()) {
			input_file >> campo;
			std::cout << campo.dato << '\n';
		}

		input_file.close();
	}

	void readFirstPage() {
		readPage(0);
	}

	void readLastPage() {
		readPage(total_pages - 1);
	}

	void readPreviousPage() {
		readPage(current_page - 1);
	}

	void readNextPage() {
		readPage(current_page + 1);
	}

	void readCurrentPage() {
		readPage(current_page);
	}

	void readAllPages() {
		//PersonTitanic::mostrarCampos();
		for (unsigned i = 0; i < total_pages; i++) { readPage(i); }
	}

	void writePage(const unsigned& current_page, const std::vector<T>& vec_records) {
		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");

		std::ofstream output_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		output_file.seekp(static_cast<size_t>(std::get<0>(map_header_page[current_page])) + PAGE_HEADER_SIZE);

		std::vector<Record<T>> vec_records_t;

		unsigned id{ 1 + records_per_pages * current_page };
		for (const auto& r : vec_records) {
			vec_records_t.push_back(Record<T>(r, id));
			id++;
		}

		const auto max_size{ vec_records_t.size() <= records_per_pages ? vec_records_t.size() : records_per_pages };

		unsigned index{ 0 };
		for (unsigned i = 0; i < max_size; i++) {
			std::get<1>(map_header_page[current_page]).bitmap.setBit(index);
			output_file << vec_records_t[i];
			index++;
		}

		output_file.close();
		setCabezalPagina(current_page);
	}

	void writePage(const unsigned& current_page, const std::vector<Record<T>>& vec_records) {
		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");

		std::ofstream output_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		output_file.seekp(static_cast<size_t>(std::get<0>(map_header_page[current_page])) + PAGE_HEADER_SIZE);

		const auto max_size{ vec_records.size() <= records_per_pages ? vec_records.size() : records_per_pages };

		unsigned index{ 0 };
		for (unsigned i = 0; i < max_size; i++) {
			std::get<1>(map_header_page[current_page]).bitmap.setBit(index);
			output_file << vec_records[i];
			index++;
		}

		output_file.close();
		setCabezalPagina(current_page);
	}

	void writePage(const unsigned& current_page, const T& record, const unsigned& record_pos = 0) {
		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");

		std::ofstream output_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		auto aux_record_pos{ record_pos };

		if (record_pos == 0) {
			const auto empty_bit_pos{ std::get<1>(map_header_page[current_page]).bitmap.getPosFirstNullBit() };
			if (empty_bit_pos == -1)
				return;
			aux_record_pos = empty_bit_pos + 1;
		}

		const auto records_pos_bytes{ getPosCampo(current_page, aux_record_pos) };

		output_file.seekp(records_pos_bytes);

		output_file << Record<T>(record, aux_record_pos + records_per_pages * current_page);

		output_file.close();

		std::get<1>(map_header_page[current_page]).bitmap.setBit(aux_record_pos - 1);
		setCabezalPagina(current_page);
		return;
	}

	void writeCurrentPage(const std::vector<T>& registro) {
		writePage(current_page, registro);
	}

	void writeCurrentPage(const T& campo, const unsigned& numero_campo = 0) {
		writePage(current_page, campo, numero_campo);
	}

	void fileInfo() const {
		std::cout << "CANTIDAD DE PAGINAS: " << total_pages << "\nCAMPOS POR PAGINA: " << records_per_pages << "\nBYTES DEL TIPO DE DATO: " << sizeof(Record<T>)
			<< "\BYTES CABEZERAL INICIAL: " << sizeof(MainHeader) << "\nBYTES CABEZERAL DE PAGINA: " << PAGE_HEADER_SIZE << "\nBYTES POR PAGINA: "
			<< PAGE_SIZE << "\nBYTES TOTALES DEL ARCHIVO: " << sizeof(MainHeader) + PAGE_SIZE * total_pages << '\n';
	}

	void PageHeaderInfo() const {
		std::for_each(map_header_page.cbegin(), map_header_page.cend(), [](const auto& n)
			{ std::cout << std::get<1>(n.second) << '\n'; });
	}
};
#include <iostream>

#include "BPlusTreeFile.h"
#include "TitanicPerson.h"
#include "LRU.h"

constexpr std::string_view FILE_TITANIC_PATH{ "personas_titanic.bin" };
constexpr unsigned NUMBER_PAGES{ 18 };
constexpr unsigned NUMBER_RECORDS{ 50 };

constexpr std::string_view FILE_B_TREE_PATH{ "b_tree_index.bin" };
constexpr unsigned DEGREE{ 3 };


void test_1(FixedLengthFile<TitanicPerson>* file) {
	std::vector<TitanicPerson> vec{};
	unsigned current_page{ 0 };

	for (unsigned i = 0; i < NUMBER_PAGES - 1; i++) {
		//std::cout << i * NUMBER_RECORDS + 1 << ' ' << (i + 1) * NUMBER_RECORDS << '\n';
		vec = TitanicPerson::getRecords(i * NUMBER_RECORDS, (i + 1) * NUMBER_RECORDS);
		file->writeCurrentPage(vec);
		current_page++;
		file->setActualPage(current_page);
	}
}


void test_data_base() {

}


int main() {

	//auto file_titanic{ FixedLengthFile<TitanicPerson>(FILE_PATH, NUMBER_PAGES, NUMBER_RECORDS) };
	auto file_titanic{ FixedLengthFile<TitanicPerson>(FILE_TITANIC_PATH) };
	

	auto b_tree{ BPlusTreeFile<unsigned, unsigned>(FILE_B_TREE_PATH, DEGREE)};

	return 0;
}




#pragma once
#include "BPlusTree.h"
#include <vector>

template<typename T, typename U>
class BPlusTreeFile
{
private:
	struct MainHeader {
		unsigned num_pages{};
		unsigned degree{};
		T root_page;
	};

	BPlusTree<T> b_tree{};
	MainHeader main_header{};

	const std::string PATH_FILE{};
	std::vector<T> vec_keys{};
	std::vector<U> vec_values{};

public:
	BPlusTreeFile(const std::string_view _PATH_FILE, const unsigned _degree)
		: PATH_FILE(_PATH_FILE) {
		main_header.degree = _degree;
	}

	BPlusTreeFile(const std::string_view _PATH_FILE) {

	}
};


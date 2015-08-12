#include <cstring>
#include <iostream>

int get_vow_offset(char const *sym, size_t size) {

	if (size != 2) return -1;

	if (!memcmp(sym, "а", size) || !memcmp(sym, "А", size)) {
		return 0;
	}
	if (!memcmp(sym, "о", size) || !memcmp(sym, "О", size)) {
		return 1;
	}
	if (!memcmp(sym, "у", size) || !memcmp(sym, "У", size)) {
		return 2;
	}
	if (!memcmp(sym, "ы", size) || !memcmp(sym, "Ы", size)) {
		return 3;
	}
	if (!memcmp(sym, "э", size) || !memcmp(sym, "Э", size)) {
		return 4;
	}
	if (!memcmp(sym, "я", size) || !memcmp(sym, "Я", size)) {
		return 5;
	}
	if (!memcmp(sym, "ё", size) || !memcmp(sym, "Ё", size)) {
		return 6;
	}
	if (!memcmp(sym, "ю", size) || !memcmp(sym, "Ю", size)) {
		return 7;
	}
	if (!memcmp(sym, "и", size) || !memcmp(sym, "И", size)) {
		return 8;
	}
	if (!memcmp(sym, "е", size) || !memcmp(sym, "Е", size)) {
		return 9;
	}

	return -1;
}


size_t get_sym_size(char const *ptr) {
	unsigned char lb = *ptr;
	size_t size = 0;

	if (( lb & 0x80 ) == 0 )          // lead bit is zero, must be a single ascii
		size = 1;
	else if (( lb & 0xE0 ) == 0xC0 )  // 110x xxxx
		size = 2;
	else if (( lb & 0xF0 ) == 0xE0 ) // 1110 xxxx
		size = 3;
	else if (( lb & 0xF8 ) == 0xF0 ) // 1111 0xxx
		size = 4;

	return size;
}

void get_swear_word(const char *str, size_t len, char **suffix, int *subst_off) {
	char const *ptr = str;
	*suffix = nullptr;
	*subst_off = -1;
	while (ptr - str < len) {
		size_t size = get_sym_size(ptr);
		if (!size) {
			break;
		}
		if (size == 2) {
			int off = get_vow_offset(ptr, size);
			if (off >= 0) {
				*suffix = (char *)(ptr + size);
				if (off == 1) {
					off = 4;
				}
				if (off < 5) {
					off += 5;
				}
				*subst_off = off;
				break;
			}
		}
		ptr += size;
	}
}

void get_next_word(const char *str, size_t len, char **word, size_t *wlen) {
	char const *ptr = str;
	*word = nullptr;
	*wlen = 0;
	while (ptr - str < len) {
		size_t size = get_sym_size(ptr);
		if (!size) {
			break;
		}
		if (size != 1 || (size == 1 && !isspace(*ptr))) {
			*word = (char *)ptr;
			break;
		}
		ptr += size;
	}

	if (!*word) {
		return;
	}

	while (ptr - str < len) {
		size_t size = get_sym_size(ptr);
		if (!size) {
			break;
		}
		if (size == 1 && isspace(*ptr)) {
			*wlen = ptr - *word;
			break;
		}
		ptr += size;
	}

	if (!*wlen) {
		*wlen = len - (*word - str);
	}
}

/*
int main() {
	char *word = nullptr;
	size_t len = 0;
	char const *str = "да желанный ты свет!";
	size_t length = 32;
	char const *ptr = str;
	size_t new_len = length;
	while (ptr < str + length) {
		get_next_word(ptr, new_len, &word, &len);
		if (!word || !len) break;
		std::cout << word << " " << len << std::endl;
		ptr = word + len;
		new_len = length - (ptr - str);
	}
	return 0;
}
*/

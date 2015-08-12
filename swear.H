#pragma once
const char* lower_vowels[10] = { ("а"), ("о"), ("у"), ("ы"), ("э"), ("я"), ("ё"), ("ю"), ("и"), ("е")  //е
};

void get_swear_word(const char *str, size_t len, char **suffix, int *subst_off);
void get_next_word(const char *str, size_t len, char **word, size_t *wlen);


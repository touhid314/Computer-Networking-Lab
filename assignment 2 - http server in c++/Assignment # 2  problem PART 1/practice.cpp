#include <stdio.h>
#include <string.h>
#include "http_helper.cpp"

#include <ctype.h>
#include <string.h>

#include <ctype.h>
#include <string.h>

void delete_spaces(char *str) {
    // find first non-whitespace character
    int first = 0;
    while (isspace(str[first])) {
        first++;
    }

    // shift characters to the left
    int i;
    for (i = 0; i <= strlen(str) - first; i++) {
        str[i] = str[i + first];
    }
}

int main() {
    char st[] = "  HTTP1.1";
    delete_spaces(st);
    printf("%s", st);

}

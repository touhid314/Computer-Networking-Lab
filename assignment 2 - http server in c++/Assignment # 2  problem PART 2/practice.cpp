#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

void stripRedundantSpaces(char* str) {
    int len = strlen(str);
    int new_pos = 0;
    bool prev_space = true; // Initially set to true to handle leading spaces

    for (int current = 0; current < len; current++) {
        if (isspace((unsigned char)str[current])) {
            if (str[current] == '\n') {
                str[new_pos++] = str[current];
                prev_space = true;
            }
            else if (!prev_space) {
                str[new_pos++] = ' ';
                prev_space = true;
            }
        } else {
            str[new_pos++] = str[current];
            prev_space = false;
        }
    }

    str[new_pos] = '\0';
}

int main() {
    char st[] = "PUT  /api   HTTP/1.1\n\nid=2&name=alim&email=alim@gmail.com";
    int id; char user_name[100]; char email[100];
    stripRedundantSpaces(st);
    int result = sscanf(st, "PUT /api HTTP/1.1\n\nid=%d&name=%[^&]&email=%s",&id, user_name, email);

    printf("%d %d %s %s", result, id, user_name, email);
}

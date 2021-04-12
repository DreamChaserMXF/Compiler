#include <stdio.h>
int main() {
    FILE* p = fopen("escapeseq.txt", "w");
    if (!p) {
        remove("escapeseq.txt");
        return -1;
    }
    char escape = '\n';
    fprintf(p, "%c", escape);
    fclose(p);
    return 0;
}
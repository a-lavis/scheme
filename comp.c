#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

typedef struct __List {
    void *first;
    struct List *rest;
} List;

typedef enum __lextypes {END, INT, ID, OPEN_P, CLOSE_P} lextypes; 
typedef enum __Bool {False, True} Bool;

typedef struct __Lexeme {
    lextypes label;
    char *text;
} Lexeme;

Bool alpha(char c) {
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        c == '_' || c == '+' || c == '-' || c == '*' || c == '/' ;
}
Bool numeric(char c) {
    return c >= '0' && c <= '9';
}
Bool alphanumeric(char c) {
    return alpha(c) || numeric(c);
}

Lexeme *lexer(char *map, size_t filesize) {
    Lexeme *lexed = calloc(1, sizeof(Lexeme));
    Lexeme *current;
    int len;
    int count = 1;
    Bool comment = False;
    char *end_of_map = map+filesize;
    for(; map < end_of_map; map++) {
        if (*map == '\n') {
            comment = False;
        } else if (*map == ';') {
            comment = True;
        } else if (!comment && *map != ' ' && *map != '\t') {
            count++;
            lexed = realloc(lexed, count * sizeof(Lexeme));
            current = &lexed[count-2];
            if (*map == '(') {
                current->label = OPEN_P;
                current->text = calloc(2, sizeof(char));
                current->text[0] = '(';
                current->text[1] = 0;
            } else if (*map == ')') {
                current->label = CLOSE_P;
                current->text = calloc(2, sizeof(char));
                current->text[0] = ')';
                current->text[1] = 0;
            } else if (alpha(*map)) { // first char is alphabetical, its a ID
                current->label = ID;
                len = 1;
                current->text = calloc(1, sizeof(char));
                for(; alphanumeric(*map); map++) {
                    len++;
                    current->text = realloc(current->text, len * sizeof(char));
                    current->text[len-2] = *map;
                }
                current->text[len-1] = 0;
                map--;
            } else if (numeric(*map)) { // first char is numerical, its a INT
                current->label = INT;
                len = 1;
                current->text = calloc(1, sizeof(char));
                for(; numeric(*map); map++) {
                    len++;
                    current->text = realloc(current->text, len * sizeof(char));
                    current->text[len-2] = *map;
                }
                current->text[len-1] = 0;
                map--;
            } else {
                fprintf(stderr, "unrecognized char\n");
                exit(1);
            }
        }
    }
    lexed[count-1].label = END;
    lexed[count-1].text = NULL;
    return lexed;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "too many or too few args\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open syscall error\n");
        exit(1);
    }
    struct stat st;
    stat(argv[1], &st);
    size_t filesize = st.st_size;

    char *mapped = (char *) mmap(NULL,
            filesize, PROT_READ, MAP_PRIVATE, fd, 0);

    Lexeme *lexed = lexer(mapped, filesize);

    munmap(mapped, filesize);
    close(fd);
    for (int i = 0; lexed[i].label != END; i++) {
        if (lexed[i].label == INT) {
            printf("INT:     %s\n", lexed[i].text);
        } else if (lexed[i].label == ID) {
            printf("ID:      %s\n", lexed[i].text);
        } else if (lexed[i].label == OPEN_P) {
            printf("OPEN_P:  %s\n", lexed[i].text);
        } else if (lexed[i].label ==CLOSE_P) {
            printf("CLOSE_P: %s\n", lexed[i].text);
        } else {
            fprintf(stderr, "aaaaaa??\n");
        }
    }
    return 0;
}

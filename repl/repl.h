
#define MAX_CHARS 512
#define MAX_ARGS 16



int tokenize(char *line, char **argv, int max_args);

void CMD_add(int argc, char **argv);
void CMD_print(int argc, char **argv);
void CMD_delete(int argc, char **argv);
void CMD_connect(int argc, char **argv);
void CMD_help(int argc, char **argv);

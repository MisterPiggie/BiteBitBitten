#include "../types/types.h"
#include <stdbool.h>

#define MAX_CHARS 512
#define MAX_ARGS 16
int tokenize(char *line, char **argv, int max_args);

void CMD_add(CL_session *session, int argc, char **argv);
void CMD_print(CL_session *session, int argc, char **argv);
void CMD_delete(CL_session *session, int argc, char **argv);
void CMD_list(CL_session *session, int argc, char **argv);
// void CMD_help(CL_session *session, int argc, char **argv);

void dispatch(CL_session *seesion, int argc, char *argv[]);

bool is_duplicate_torrent(uint8_t hash[20], CL_session *session);


typedef void (*CMD_fn)(CL_session *session, int argc, char **argv);

typedef struct {
    const char *name;
    CMD_fn     fn;
} CMD_command;

static const CMD_command CMD_commands[] =
{
    {"add", CMD_add},
    {"print", CMD_print},
    {"delete", CMD_delete},
    {"list", CMD_list},
    // {"help", CMD_help},
};

static const int NUM_COMMANDS = sizeof(CMD_commands) / sizeof(CMD_commands[0]);


int id_from_arg(const char *str, int *out);

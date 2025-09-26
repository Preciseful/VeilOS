#include <svc.h>

static char *initial_env[1] = {0};
char **environ = initial_env;

void _set_environ()
{
    svc_call(&environ, 0, 0, 0, 0, 0, 0, 0, SYS_SET_ENVIRON);
}
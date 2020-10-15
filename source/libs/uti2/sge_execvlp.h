#ifndef __SGE_EXECVLP_H
#define __SGE_EXECVLP_H
#include "uti/sge_uidgid.h"
int sge_execvlp (const char *file, char *const argv[], char *const envp[]);
char **sge_copy_sanitize_env (char *const env[]);
void sanitize_environment (char *env[]);
#endif  /* __SGE_EXECVLP_H */

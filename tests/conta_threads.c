#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*CreateFn)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);

static unsigned long created;

/**
 * Intercepta pthread_create para contar as chamadas. Com FALHAR_THREADS no
 * ambiente, nenhuma thread e criada e a chamada devolve EAGAIN.
 */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg)
{
    static CreateFn real;

    if (real == NULL) {
        real = (CreateFn)dlsym(RTLD_NEXT, "pthread_create");
    }
    if (getenv("FALHAR_THREADS") != NULL) {
        return EAGAIN;
    }
    __atomic_fetch_add(&created, 1, __ATOMIC_RELAXED);
    return real(thread, attr, start, arg);
}

/** Ao fim do programa, grava no arquivo de CONTA_THREADS_SAIDA o total de threads criadas. */
__attribute__((destructor)) static void report(void)
{
    const char *path = getenv("CONTA_THREADS_SAIDA");
    FILE *file = path != NULL ? fopen(path, "w") : NULL;

    if (file != NULL) {
        fprintf(file, "%lu\n", created);
        fclose(file);
    }
}

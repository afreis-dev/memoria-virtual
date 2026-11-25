#include "backing_store.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "address.h"

#define BACKING_STORE_SIZE ((long long)PAGE_COUNT * PAGE_SIZE)

int backing_store_open(BackingStore *store, const char *path, char *err, size_t err_size)
{
    struct stat info;

    store->file = NULL;
    if (stat(path, &info) != 0) {
        if (errno == ENOENT) {
            snprintf(err, err_size, "%s nao encontrado (deve estar no diretorio atual)", path);
        } else {
            snprintf(err, err_size, "nao foi possivel acessar '%s': %s", path, strerror(errno));
        }
        return -1;
    }
    if (!S_ISREG(info.st_mode)) {
        snprintf(err, err_size, "'%s' nao e um arquivo regular", path);
        return -1;
    }
    if ((long long)info.st_size != BACKING_STORE_SIZE) {
        snprintf(err, err_size, "'%s' deve ter %lld bytes, mas tem %lld", path, BACKING_STORE_SIZE,
                 (long long)info.st_size);
        return -1;
    }
    store->file = fopen(path, "rb");
    if (store->file == NULL) {
        snprintf(err, err_size, "nao foi possivel abrir '%s': %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

int backing_store_read(BackingStore *store, uint8_t page, uint8_t *buffer, char *err, size_t err_size)
{
    long position = (long)page * PAGE_SIZE;

    if (fseek(store->file, position, SEEK_SET) != 0 || fread(buffer, 1, PAGE_SIZE, store->file) != PAGE_SIZE) {
        snprintf(err, err_size, "falha ao ler a pagina %u da memoria secundaria", (unsigned)page);
        return -1;
    }
    return 0;
}

void backing_store_close(BackingStore *store)
{
    if (store->file != NULL) {
        fclose(store->file);
        store->file = NULL;
    }
}

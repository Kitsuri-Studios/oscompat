#include "wdl_ns_patch.h"

#if defined(__ANDROID__)

#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

bool wdl_ns_patch_soname(const char *path, int fd, const char *patch) {
    struct stat st;
    if (stat(path, &st) != 0 || ftruncate(fd, st.st_size) != 0) return false;

    uint8_t *map = mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) return false;

    int src = open(path, O_RDONLY);
    if (src < 0 || read(src, map, (size_t)st.st_size) != st.st_size) {
        if (src >= 0) close(src);
        munmap(map, (size_t)st.st_size);
        return false;
    }
    close(src);

    ElfW(Ehdr) *eh = (ElfW(Ehdr) *)map;
    if (eh->e_ident[EI_MAG0] != ELFMAG0) {
        munmap(map, (size_t)st.st_size);
        return false;
    }

    ElfW(Shdr) *sh = (ElfW(Shdr) *)(map + eh->e_shoff);
    for (uint16_t i = 0; i < eh->e_shnum; i++) {
        if (sh[i].sh_type != SHT_DYNAMIC) continue;
        char *str = (char *)(map + sh[sh[i].sh_link].sh_offset);
        ElfW(Dyn) *dyn = (ElfW(Dyn) *)(map + sh[i].sh_offset);
        size_t n = sh[i].sh_size / sh[i].sh_entsize;
        for (size_t k = 0; k < n; k++) {
            if (dyn[k].d_tag != DT_SONAME) continue;
            char *soname = str + dyn[k].d_un.d_val;
            for (size_t c = 0; patch[c] && soname[c]; c++) soname[c] = patch[c];
            munmap(map, (size_t)st.st_size);
            return true;
        }
    }
    munmap(map, (size_t)st.st_size);
    return false;
}

#else

bool wdl_ns_patch_soname(const char *path, int fd, const char *patch) {
    (void)path;
    (void)fd;
    (void)patch;
    return false;
}

#endif

#include <stdlib.h>
#include "zgdbindex.h"

uint64_t createIndex(zgdbFile* file) {
    return createIndexes(file, 1, false);
}

uint64_t createIndexes(zgdbFile* file, uint64_t n, bool expand) {
    munmap(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex));
    zgdbIndex index[n];
    for (int i = 0; i < n; ++i) {
        index[i].flag = INDEX_NEW;
        index[i].offset = 0;
    }
    off_t offset = (long long) (sizeof(zgdbHeader) + (file->zgdbHeader.indexCount) * sizeof(zgdbIndex));
    fseeko(file->file, offset, SEEK_SET);
    fwrite(&index, sizeof(zgdbIndex), n, file->file);
    file->zgdbHeader.indexCount += n;
    if(!expand)
        file->zgdbHeader.fileSize += (off_t) (n * sizeof(zgdbIndex));
    saveHeader(file);
    file->pIndexesMmap = (char *) mmap(NULL, file->zgdbHeader.indexCount * sizeof(zgdbIndex),
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        fileno(file->file), 0) + sizeof(zgdbHeader);
    return 0;
}

zgdbIndex getIndex(zgdbFile* file, uint64_t order) {
    zgdbIndex index;
    if (order > file->zgdbHeader.indexCount - 1) {
        index.flag = INDEX_INVALID;
        index.offset = 0;
        return index;
    }
    zgdbIndex* pIndex = (zgdbIndex*) file->pIndexesMmap;
    index = pIndex[order];
    return index;
}

bool attachIndexToBlock(zgdbFile* file, uint64_t order, off_t blockOffset) {
    if (order > file->zgdbHeader.indexCount - 1) {
        return false;
    }
    zgdbIndex* pIndex = (zgdbIndex*) file->pIndexesMmap;
    pIndex[order].flag = INDEX_ALIVE;
    pIndex[order].offset = blockOffset;
    msync(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex), MS_ASYNC);
    return true;
}

bool killIndex(zgdbFile* file, uint64_t order) {
    if (order > file->zgdbHeader.indexCount - 1) {
        return false;
    }
    zgdbIndex* pIndex = (zgdbIndex*) file->pIndexesMmap;
    if(pIndex[order].flag != INDEX_ALIVE)
        return false;
    pIndex[order].flag = INDEX_DEAD;
    msync(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex), MS_ASYNC);
    return true;
}

bool updateOffset(zgdbFile* file, uint64_t indexOrder, off_t offset) {
    if (indexOrder > file->zgdbHeader.indexCount - 1) {
        return false;
    }
    zgdbIndex* pIndex = (zgdbIndex*) file->pIndexesMmap;
    pIndex[indexOrder].offset = offset;
    msync(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex), MS_ASYNC);
    return true;
}
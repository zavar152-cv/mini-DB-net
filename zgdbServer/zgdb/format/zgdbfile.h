#ifndef ZGDBPROJECT_ZGDBFILE_H
#define ZGDBPROJECT_ZGDBFILE_H
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "index/list/freelist.h"
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifdef __MINGW32__
#include "mman-win32/mman.h"
#endif

/*
 * Структура для заголовка файла, где храниться мета информация
 * zgdbType - 4 символа в UTF8 (ZGDB)
 * version - версия типа
 * indexCount - количество всех индексов (5 байт)
 * betweenSpace - пространство между первым блоком и последним индексом
 * fileSize - размер файла (с учётом заголовка), должен быть 8 байт
 * freeListOffset - смещение на структуру с доступными индексами (reserved)
 */
typedef struct zgdbHeader {
    uint32_t zgdbType;
    uint8_t version;
    uint64_t indexCount: 40;
    uint8_t betweenSpace;
    //uint64_t freeListOffset;
    uint64_t nodes;
    uint64_t fileSize;
} zgdbHeader;

/*
 * Структура для представления zgdb файла
 */
typedef struct zgdbFile {
    FILE* file;
    void* pIndexesMmap;
    freeIndexesList freeList;
    zgdbHeader zgdbHeader;
} zgdbFile;

/*
 * Загрузка/создание zgdb файла
 */
zgdbFile* loadOrCreateZgdbFile(const char* path);

/*
 * Закрытие zgdb файла
 */
bool closeZgdbFile(zgdbFile* file);

/*
 * Сохранение информации в заголовке на диск
 */
void saveHeader(zgdbFile* file);

#endif

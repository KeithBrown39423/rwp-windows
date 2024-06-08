#pragma once
#ifndef _RWP_H
#define _RWP_H

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

char* getProcessName(uint32_t pid);
bool rwpOpenHandle(uint32_t pid, HANDLE* process);
bool rwpCloseHandle(HANDLE process);
bool rwpReadMemory(HANDLE process, unsigned long long baseAddress, void* buffer, size_t size);
bool rwpWriteMemory(HANDLE process, unsigned long long baseAddress, void* buffer, size_t size);
size_t rwpGetOpenProcessCount();
bool rwpGetRunningProcessPIDs(uint32_t* processList, size_t amount);
bool rwpGetRunningProcessNames(char** processList, size_t amount);
bool rwpNameToPID(uint32_t* pid, char* procName);

#endif
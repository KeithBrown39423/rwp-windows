#include <stdio.h>

#define _WIN32_WINNT 0x0501 // Windows XP
#define PATH_DELIMITER '\\'

#pragma comment(lib, "psapi.lib") // Visual Studio - Add psapi.lib
#include <Windows.h>
#include <Psapi.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "memutils.h"

char* getProcessName(uint32_t pid) {
    char* processName = xmalloc(MAX_PATH + 1); // + 1 for null terminator

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, false, (DWORD)pid);
    if (process == NULL) {
        free(processName);
        return NULL;
    }

    DWORD processNameLength = GetProcessImageFileNameA(process, processName, MAX_PATH + 1);

    CloseHandle(process);

    if (processNameLength == 0) {
        free(processName);
        return NULL;
    }

    char* filename = strrchr(processName, PATH_DELIMITER);
    if (filename == NULL) {
        return processName;
    }
    else {
        char* ret = strdup(filename + 1); // Skip the separator and return the filename
        free(processName);
        return ret;
    }

    // Failsafe, should be unreachable
    return NULL;
}

bool rwpOpenHandle(uint32_t pid, HANDLE* process) {
    DWORD desiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION;
    bool inheritHandle = true;
    *process = OpenProcess(desiredAccess, inheritHandle, pid);
    return *process != NULL;
}

bool rwpCloseHandle(HANDLE process) {
    return CloseHandle(process);
}

bool rwpReadMemory(HANDLE process, unsigned long long baseAddress, void* buffer, size_t size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(process, baseAddress, buffer, size, &bytesRead);
}

bool rwpWriteMemory(HANDLE process, unsigned long long baseAddress, void* buffer, size_t size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(process, baseAddress, buffer, size, &bytesWritten);
}

size_t rwpGetOpenProcessCount() {
    DWORD* processList = xmalloc(256 * sizeof(DWORD));
    DWORD processListSize = 256 * sizeof(DWORD);
    DWORD bytesReturned = 256 * sizeof(DWORD);

    // Loop until we get all the process IDs
    // Loop returns when we have all the process IDs
    // so no need to specify a loop condition
    while (1) {
        if (!EnumProcesses(processList, processListSize, &bytesReturned)) {
            return 0;
        }

        if (bytesReturned == processListSize) {
            processList = xrealloc(processList, processListSize * 2);
            processListSize += 256;
        }
        else {
            free(processList);
            return (size_t)(bytesReturned / sizeof(DWORD));
        }
    }
}

// amount is the number of DWORDs in processList, i.e. the number of processes
bool rwpGetRunningProcessPIDs(uint32_t* processList, size_t amount) {
    DWORD bytesReturned;
    if (!EnumProcesses(processList, (DWORD)amount * sizeof(DWORD), &bytesReturned)) {
        fprintf(stderr, "Failed to get process list\n");
        return false;
    }

    // Check if the process IDs are valid
    // If not, set the value to 0 so we can skip it later
    for (size_t i = 0; i < amount; i++) {
        if (processList[i] == 0) {
            continue;
        }

        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, false, processList[i]);
        if (process == NULL) {
            processList[i] = 0;
            continue;
        }
        CloseHandle(process);
    }

    return true;
}

bool rwpGetRunningProcessNames(char** processList, size_t amount) {
    uint32_t* processIDs = xmalloc(amount * sizeof(uint32_t));
    if (!rwpGetRunningProcessPIDs(processIDs, amount)) {
        free(processIDs);
        return false;
    }

    for (size_t i = 0; i < amount; i++) {
        if (processIDs[i] == 0) {
            processList[i] = NULL;
            continue;
        }

        char* processName = getProcessName(processIDs[i]);
        if (processName == NULL) {
            continue;
        }

        processList[i] = processName;
    }

    free(processIDs);

    return true;
}

bool rwpNameToPID(uint32_t* pid, char* procName) {
    size_t processCount = rwpGetOpenProcessCount();
    if (processCount == 0) {
        fprintf(stderr, "Failed to get process count\n");
        return false;
    }
    uint32_t* processList = xmalloc(processCount * sizeof(uint32_t));

    if (!rwpGetRunningProcessPIDs(processList, processCount)) {
        fprintf(stderr, "Failed to get process IDs\n");
        free(processList);
        return false;
    }

    for (size_t i = 0; i < processCount; i++) {
        if (processList[i] == 0) {
            continue;
        }

        char* processName = getProcessName(processList[i]);
        if (processName == NULL) {
            continue;
        }

        if (strcmp(processName, procName) == 0) {
            *pid = processList[i];
            free(processName);
            free(processList);
            return true;
        }

        free(processName);
    }

    return false;
}

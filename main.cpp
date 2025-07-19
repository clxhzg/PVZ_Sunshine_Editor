#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <conio.h>

// 阳光内存地址相关偏移量
#define BASE_OFFSET 0x00355E0C
#define SUN_POINTER_OFFSET 0x868
#define SUN_VALUE_OFFSET 0x5578

// 函数声明
void ClearScreen();
DWORD GetProcessIdByName(const wchar_t* processName);
DWORD GetSunValue(HANDLE handle, DWORD baseAddr);
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue);
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue);

int main() {
    const wchar_t* processName = L"PlantsVsZombies.exe";
    DWORD processId;
    HANDLE hProcess;
    DWORD baseAddress = 0x00400000; // 游戏基地址
    DWORD newSunValue;
    int choice;

    // 获取进程ID
    processId = GetProcessIdByName(processName);
    if (processId == 0) {
        ClearScreen();
        wprintf(L"错误: 未找到 %s 进程。请先运行游戏。\n", processName);
        system("pause");
        return 1;
    }

    // 打开进程
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        ClearScreen();
        printf("错误: 无法打开进程。请以管理员身份运行此程序。\n");
        system("pause");
        return 1;
    }

    while (1) {
        ClearScreen();
        printf("植物大战僵尸阳光修改器\n");
        printf("----------------------------\n");
        printf("\n当前阳光值: %u\n", GetSunValue(hProcess, baseAddress));
        printf("\n1. 修改阳光值\n");
        printf("2. 持续锁定阳光值\n");
        printf("3. 退出\n");
        printf("请选择: ");

        // 使用安全的scanf_s
        if (scanf_s("%d", &choice) != 1) {
            while (getchar() != '\n'); // 清除输入缓冲区
            printf("\n输入无效，请重新选择!\n");
            Sleep(1000);
            continue;
        }

        switch (choice) {
        case 1: {
            ClearScreen();
            DWORD currentSun = GetSunValue(hProcess, baseAddress);
            printf("当前阳光值: %u\n", currentSun);
            printf("\n请输入新的阳光值: ");

            if (scanf_s("%u", &newSunValue) != 1) {
                while (getchar() != '\n'); // 清除输入缓冲区
                printf("\n输入无效!\n");
            }
            else if (SetSunValue(hProcess, baseAddress, newSunValue)) {
                printf("\n阳光值已成功修改为 %u\n", newSunValue);
            }
            else {
                printf("\n修改失败!\n");
            }
            system("pause");
            break;
        }

        case 2: {
            ClearScreen();
            printf("当前阳光值: %u\n", GetSunValue(hProcess, baseAddress));
            printf("\n请输入要锁定的阳光值: ");

            if (scanf_s("%u", &newSunValue) != 1) {
                while (getchar() != '\n'); // 清除输入缓冲区
                printf("\n输入无效!\n");
                system("pause");
                break;
            }

            ClearScreen();
            printf("正在持续锁定阳光值为 %u (按任意键停止)...\n", newSunValue);
            MonitorSunValue(hProcess, baseAddress, newSunValue);
            printf("\n已停止锁定阳光值\n");
            system("pause");
            break;
        }

        case 3:
            CloseHandle(hProcess);
            return 0;

        default:
            printf("\n无效选择!\n");
            Sleep(1000);
        }
    }
}

// 清屏函数
void ClearScreen() {
    system("cls");
}

// 通过进程名获取进程ID
DWORD GetProcessIdByName(const wchar_t* processName) {
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (wcscmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

// 获取阳光值
DWORD GetSunValue(HANDLE handle, DWORD baseAddr) {
    DWORD value = 0;
    DWORD_PTR addr = baseAddr + BASE_OFFSET;

    // 读取指针链
    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return 0;
    }
    addr += SUN_POINTER_OFFSET;

    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return 0;
    }
    addr += SUN_VALUE_OFFSET;

    if (!ReadProcessMemory(handle, (LPCVOID)addr, &value, sizeof(DWORD), NULL)) {
        return 0;
    }

    return value;
}

// 设置阳光值
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue) {
    DWORD_PTR addr = baseAddr + BASE_OFFSET;

    // 读取指针链
    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return FALSE;
    }
    addr += SUN_POINTER_OFFSET;

    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return FALSE;
    }
    addr += SUN_VALUE_OFFSET;

    // 写入新值
    return WriteProcessMemory(handle, (LPVOID)addr, &newValue, sizeof(DWORD), NULL);
}

// 持续监控并锁定阳光值
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue) {
    while (!_kbhit()) {
        if (GetSunValue(handle, baseAddr) != targetValue) {
            SetSunValue(handle, baseAddr, targetValue);
        }
        Sleep(100);
    }
    _getch();
}
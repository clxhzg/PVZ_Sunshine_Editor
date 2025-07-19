#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <conio.h>

// �����ڴ��ַ���ƫ����
#define BASE_OFFSET 0x00355E0C
#define SUN_POINTER_OFFSET 0x868
#define SUN_VALUE_OFFSET 0x5578

// ��������
void ClearScreen();
DWORD GetProcessIdByName(const wchar_t* processName);
DWORD GetSunValue(HANDLE handle, DWORD baseAddr);
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue);
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue);

int main() {
    const wchar_t* processName = L"PlantsVsZombies.exe";
    DWORD processId;
    HANDLE hProcess;
    DWORD baseAddress = 0x00400000; // ��Ϸ����ַ
    DWORD newSunValue;
    int choice;

    // ��ȡ����ID
    processId = GetProcessIdByName(processName);
    if (processId == 0) {
        ClearScreen();
        wprintf(L"����: δ�ҵ� %s ���̡�����������Ϸ��\n", processName);
        system("pause");
        return 1;
    }

    // �򿪽���
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        ClearScreen();
        printf("����: �޷��򿪽��̡����Թ���Ա������д˳���\n");
        system("pause");
        return 1;
    }

    while (1) {
        ClearScreen();
        printf("ֲ���ս��ʬ�����޸���\n");
        printf("----------------------------\n");
        printf("\n��ǰ����ֵ: %u\n", GetSunValue(hProcess, baseAddress));
        printf("\n1. �޸�����ֵ\n");
        printf("2. ������������ֵ\n");
        printf("3. �˳�\n");
        printf("��ѡ��: ");

        // ʹ�ð�ȫ��scanf_s
        if (scanf_s("%d", &choice) != 1) {
            while (getchar() != '\n'); // ������뻺����
            printf("\n������Ч��������ѡ��!\n");
            Sleep(1000);
            continue;
        }

        switch (choice) {
        case 1: {
            ClearScreen();
            DWORD currentSun = GetSunValue(hProcess, baseAddress);
            printf("��ǰ����ֵ: %u\n", currentSun);
            printf("\n�������µ�����ֵ: ");

            if (scanf_s("%u", &newSunValue) != 1) {
                while (getchar() != '\n'); // ������뻺����
                printf("\n������Ч!\n");
            }
            else if (SetSunValue(hProcess, baseAddress, newSunValue)) {
                printf("\n����ֵ�ѳɹ��޸�Ϊ %u\n", newSunValue);
            }
            else {
                printf("\n�޸�ʧ��!\n");
            }
            system("pause");
            break;
        }

        case 2: {
            ClearScreen();
            printf("��ǰ����ֵ: %u\n", GetSunValue(hProcess, baseAddress));
            printf("\n������Ҫ����������ֵ: ");

            if (scanf_s("%u", &newSunValue) != 1) {
                while (getchar() != '\n'); // ������뻺����
                printf("\n������Ч!\n");
                system("pause");
                break;
            }

            ClearScreen();
            printf("���ڳ�����������ֵΪ %u (�������ֹͣ)...\n", newSunValue);
            MonitorSunValue(hProcess, baseAddress, newSunValue);
            printf("\n��ֹͣ��������ֵ\n");
            system("pause");
            break;
        }

        case 3:
            CloseHandle(hProcess);
            return 0;

        default:
            printf("\n��Чѡ��!\n");
            Sleep(1000);
        }
    }
}

// ��������
void ClearScreen() {
    system("cls");
}

// ͨ����������ȡ����ID
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

// ��ȡ����ֵ
DWORD GetSunValue(HANDLE handle, DWORD baseAddr) {
    DWORD value = 0;
    DWORD_PTR addr = baseAddr + BASE_OFFSET;

    // ��ȡָ����
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

// ��������ֵ
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue) {
    DWORD_PTR addr = baseAddr + BASE_OFFSET;

    // ��ȡָ����
    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return FALSE;
    }
    addr += SUN_POINTER_OFFSET;

    if (!ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL)) {
        return FALSE;
    }
    addr += SUN_VALUE_OFFSET;

    // д����ֵ
    return WriteProcessMemory(handle, (LPVOID)addr, &newValue, sizeof(DWORD), NULL);
}

// ������ز���������ֵ
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue) {
    while (!_kbhit()) {
        if (GetSunValue(handle, baseAddr) != targetValue) {
            SetSunValue(handle, baseAddr, targetValue);
        }
        Sleep(100);
    }
    _getch();
}
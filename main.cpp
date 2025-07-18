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
DWORD GetProcessIdByName(const char* processName);
DWORD GetSunValue(HANDLE handle, DWORD baseAddr);
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue);
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue);

int main() {
    const char* processName = "PlantsVsZombies.exe";
    DWORD processId;
    HANDLE hProcess;
    DWORD baseAddress = 0x00400000; // ��Ϸ����ַ
    DWORD newSunValue;
    int choice;

    // ��ȡ����ID
    processId = GetProcessIdByName(processName);
    if (processId == 0) {
        ClearScreen();
        printf("����: δ�ҵ� %s ���̡�����������Ϸ��\n", processName);
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
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                ClearScreen();
                printf("��ǰ����ֵ: %u\n", GetSunValue(hProcess, baseAddress));
                printf("\n�������µ�����ֵ: ");
                scanf("%u", &newSunValue);
                if (SetSunValue(hProcess, baseAddress, newSunValue)) {
                    printf("\n����ֵ�ѳɹ��޸�Ϊ %u\n", newSunValue);
                } else {
                    printf("\n�޸�ʧ��!\n");
                }
                system("pause");
                break;
            
            case 2:
                ClearScreen();
                printf("��ǰ����ֵ: %u\n", GetSunValue(hProcess, baseAddress));
                printf("\n������Ҫ����������ֵ: ");
                scanf("%u", &newSunValue);
                ClearScreen();
                printf("���ڳ�����������ֵΪ %u (�������ֹͣ)...\n", newSunValue);
                MonitorSunValue(hProcess, baseAddress, newSunValue);
                printf("\n��ֹͣ��������ֵ\n");
                system("pause");
                break;
            
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
    system("cls");  // Windowsϵͳʹ��cls����
}

// ͨ����������ȡ����ID
DWORD GetProcessIdByName(const char* processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return 0;
}

// ��ȡ����ֵ
DWORD GetSunValue(HANDLE handle, DWORD baseAddr) {
    DWORD value = 0;
    DWORD_PTR addr = baseAddr + BASE_OFFSET;
    
    // ��ȡָ����
    ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL);
    addr += SUN_POINTER_OFFSET;
    ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL);
    addr += SUN_VALUE_OFFSET;
    ReadProcessMemory(handle, (LPCVOID)addr, &value, sizeof(DWORD), NULL);
    
    return value;
}

// ��������ֵ
BOOL SetSunValue(HANDLE handle, DWORD baseAddr, DWORD newValue) {
    DWORD_PTR addr = baseAddr + BASE_OFFSET;
    
    // ��ȡָ����
    ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL);
    addr += SUN_POINTER_OFFSET;
    ReadProcessMemory(handle, (LPCVOID)addr, &addr, sizeof(DWORD_PTR), NULL);
    addr += SUN_VALUE_OFFSET;
    
    // д����ֵ
    return WriteProcessMemory(handle, (LPVOID)addr, &newValue, sizeof(DWORD), NULL);
}

// ������ز���������ֵ
void MonitorSunValue(HANDLE handle, DWORD baseAddr, DWORD targetValue) {
    while (!_kbhit()) {  // ����Ƿ��а�������
        if (GetSunValue(handle, baseAddr) != targetValue) {
            SetSunValue(handle, baseAddr, targetValue);
        }
        Sleep(100);  // ÿ100������һ��
    }
    _getch();  // �������������
}

#include "protection.h"
#include "obfuscation.h"
#include <intrin.h>
#include <TlHelp32.h>
#include <iphlpapi.h>
#include <winternl.h>

#pragma comment(lib, "iphlpapi.lib")

// ============================================================================
// PEB STRUCTURE DEFINITION (for BeingDebugged access)
// ============================================================================

// Manuel PEB tanımı (sadece ihtiyacımız olan alanlar)
typedef struct _MY_PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PVOID Ldr;
    PVOID ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    PVOID PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} MY_PEB, *PMY_PEB;

// NtQueryInformationProcess typedef
typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
);

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static uint32_t g_OriginalCRC = 0;
static bool g_CRCInitialized = false;

// ============================================================================
// ANTI-DEBUG IMPLEMENTATION
// ============================================================================

namespace AntiDebug {

    bool IsDebuggerPresentCheck() {
        JUNK_CODE_1;
        return ::IsDebuggerPresent() != FALSE;
    }

    bool CheckRemoteDebugger() {
        JUNK_CODE_2;
        BOOL isDebuggerPresent = FALSE;
        if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent)) {
            return isDebuggerPresent != FALSE;
        }
        return false;
    }

    bool CheckDebugPort() {
        JUNK_CODE_1;
        
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (!hNtdll) return false;

        pNtQueryInformationProcess NtQueryInformationProcess = 
            (pNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
        
        if (!NtQueryInformationProcess) return false;

        DWORD_PTR debugPort = 0;
        NTSTATUS status = NtQueryInformationProcess(
            GetCurrentProcess(),
            7, // ProcessDebugPort
            &debugPort,
            sizeof(debugPort),
            nullptr
        );

        JUNK_CODE_2;
        return (status == 0 && debugPort != 0);
    }

    bool CheckPEBFlag() {
        JUNK_CODE_3;
        
#ifdef _WIN64
        // x64: PEB at GS:[0x60]
        PMY_PEB pPeb = (PMY_PEB)__readgsqword(0x60);
#else
        // x86: PEB at FS:[0x30]
        PMY_PEB pPeb = (PMY_PEB)__readfsdword(0x30);
#endif
        return pPeb->BeingDebugged != 0;
    }

    bool CheckNtGlobalFlag() {
        JUNK_CODE_1;
        
#ifdef _WIN64
        PMY_PEB pPeb = (PMY_PEB)__readgsqword(0x60);
        DWORD ntGlobalFlag = *(DWORD*)((PBYTE)pPeb + 0xBC);
#else
        PMY_PEB pPeb = (PMY_PEB)__readfsdword(0x30);
        DWORD ntGlobalFlag = *(DWORD*)((PBYTE)pPeb + 0x68);
#endif
        
        // FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS
        const DWORD debugFlags = 0x70;
        
        return (ntGlobalFlag & debugFlags) != 0;
    }

    bool CheckHardwareBreakpoints() {
        JUNK_CODE_2;
        
        CONTEXT ctx = { 0 };
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        
        if (GetThreadContext(GetCurrentThread(), &ctx)) {
            if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
                return true;
            }
        }
        
        return false;
    }

    bool CheckTimingRDTSC() {
        JUNK_CODE_1;
        
        volatile unsigned __int64 start = __rdtsc();
        
        // Bazı işlemler yap
        volatile int dummy = 0;
        for (volatile int i = 0; i < 1000; ++i) {
            dummy += i;
        }
        
        volatile unsigned __int64 end = __rdtsc();
        volatile unsigned __int64 diff = end - start;
        
        // Debugger altında bu değer çok yüksek olur
        // Normal: ~10000-50000, Debug: ~500000+
        const unsigned __int64 threshold = 200000;
        
        JUNK_CODE_2;
        return diff > threshold;
    }

    bool CheckTimingQPC() {
        LARGE_INTEGER freq, start, end;
        
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        
        // Bazı işlemler yap
        volatile int dummy = 0;
        for (volatile int i = 0; i < 5000; ++i) {
            dummy += i * 2;
        }
        
        QueryPerformanceCounter(&end);
        
        double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0;
        
        // Normal: ~0.1ms, Debug: ~10ms+
        return elapsed > 5.0;
    }

    bool RunAllChecks() {
        JUNK_CODE_3;
        
        if (IsDebuggerPresentCheck()) return true;
        if (CheckRemoteDebugger()) return true;
        if (CheckDebugPort()) return true;
        if (CheckPEBFlag()) return true;
        if (CheckNtGlobalFlag()) return true;
        if (CheckHardwareBreakpoints()) return true;
        if (CheckTimingRDTSC()) return true;
        if (CheckTimingQPC()) return true;
        
        return false;
    }
}

// ============================================================================
// ANTI-VM IMPLEMENTATION
// ============================================================================

namespace AntiVM {

    bool CheckCPUIDHypervisor() {
        JUNK_CODE_1;
        
        int cpuInfo[4] = { 0 };
        __cpuid(cpuInfo, 1);
        
        // Bit 31 of ECX = Hypervisor present bit
        return (cpuInfo[2] & (1 << 31)) != 0;
    }

    bool CheckRegistryKeys() {
        JUNK_CODE_2;
        
        const char* vmKeys[] = {
            "SOFTWARE\\VMware, Inc.\\VMware Tools",
            "SOFTWARE\\Oracle\\VirtualBox Guest Additions",
            "SOFTWARE\\Microsoft\\Virtual Machine\\Guest\\Parameters",
            "SYSTEM\\CurrentControlSet\\Services\\VBoxGuest",
            "SYSTEM\\CurrentControlSet\\Services\\vmci"
        };

        for (const auto& key : vmKeys) {
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return true;
            }
        }

        return false;
    }

    bool CheckMACAddress() {
        JUNK_CODE_1;
        
        // VM MAC address prefixes
        const BYTE vmMacPrefixes[][3] = {
            { 0x00, 0x0C, 0x29 }, // VMware
            { 0x00, 0x50, 0x56 }, // VMware
            { 0x08, 0x00, 0x27 }, // VirtualBox
            { 0x00, 0x1C, 0x42 }, // Parallels
            { 0x00, 0x15, 0x5D }, // Hyper-V
        };

        ULONG bufferSize = 0;
        GetAdaptersInfo(nullptr, &bufferSize);
        
        if (bufferSize == 0) return false;
        
        PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(bufferSize);
        if (!pAdapterInfo) return false;

        if (GetAdaptersInfo(pAdapterInfo, &bufferSize) == ERROR_SUCCESS) {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            while (pAdapter) {
                for (const auto& prefix : vmMacPrefixes) {
                    if (pAdapter->AddressLength >= 3 &&
                        pAdapter->Address[0] == prefix[0] &&
                        pAdapter->Address[1] == prefix[1] &&
                        pAdapter->Address[2] == prefix[2]) {
                        free(pAdapterInfo);
                        return true;
                    }
                }
                pAdapter = pAdapter->Next;
            }
        }

        free(pAdapterInfo);
        return false;
    }

    bool CheckVMProcesses() {
        JUNK_CODE_3;
        
        const char* vmProcesses[] = {
            "vmtoolsd.exe",
            "vmwaretray.exe",
            "vmwareuser.exe",
            "VBoxService.exe",
            "VBoxTray.exe",
            "vmusrvc.exe",
            "vmsrvc.exe",
            "xenservice.exe"
        };

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                for (const auto& vmProc : vmProcesses) {
                    if (_stricmp(pe32.szExeFile, vmProc) == 0) {
                        CloseHandle(hSnapshot);
                        return true;
                    }
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
        return false;
    }

    bool RunAllChecks() {
        JUNK_CODE_1;
        
        if (CheckCPUIDHypervisor()) return true;
        if (CheckRegistryKeys()) return true;
        if (CheckMACAddress()) return true;
        if (CheckVMProcesses()) return true;
        
        return false;
    }
}

// ============================================================================
// INTEGRITY CHECK IMPLEMENTATION
// ============================================================================

namespace Integrity {

    // CRC32 lookup table
    static uint32_t crc32Table[256];
    static bool tableInitialized = false;

    static void InitCRC32Table() {
        if (tableInitialized) return;
        
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (-(int)(crc & 1) & 0xEDB88320);
            }
            crc32Table[i] = crc;
        }
        tableInitialized = true;
    }

    uint32_t CalculateTextSectionCRC() {
        InitCRC32Table();
        
        HMODULE hModule = GetModuleHandle(nullptr);
        if (!hModule) return 0;

        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

        // .text section'ı bul
        for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
            if (strcmp((char*)sectionHeader[i].Name, ".text") == 0) {
                BYTE* sectionStart = (BYTE*)hModule + sectionHeader[i].VirtualAddress;
                DWORD sectionSize = sectionHeader[i].Misc.VirtualSize;

                uint32_t crc = 0xFFFFFFFF;
                for (DWORD j = 0; j < sectionSize; ++j) {
                    crc = crc32Table[(crc ^ sectionStart[j]) & 0xFF] ^ (crc >> 8);
                }
                return crc ^ 0xFFFFFFFF;
            }
        }

        return 0;
    }

    bool VerifyChecksum(uint32_t expectedCRC) {
        JUNK_CODE_1;
        return CalculateTextSectionCRC() == expectedCRC;
    }

    void InitializeChecksum() {
        g_OriginalCRC = CalculateTextSectionCRC();
        g_CRCInitialized = true;
    }

    bool RuntimeCheck() {
        if (!g_CRCInitialized) return true; // Henüz initialize edilmemişse atla
        JUNK_CODE_2;
        return VerifyChecksum(g_OriginalCRC);
    }
}

// ============================================================================
// SELF-MODIFYING CODE IMPLEMENTATION
// ============================================================================

namespace SMC {

    void EncryptFunction(void* funcPtr, size_t size, uint8_t key) {
        DWORD oldProtect;
        if (VirtualProtect(funcPtr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            uint8_t* bytes = (uint8_t*)funcPtr;
            for (size_t i = 0; i < size; ++i) {
                bytes[i] ^= key ^ (uint8_t)(i & 0xFF);
            }
            VirtualProtect(funcPtr, size, oldProtect, &oldProtect);
        }
    }

    void DecryptFunction(void* funcPtr, size_t size, uint8_t key) {
        // XOR şifreleme simetrik olduğu için aynı fonksiyon
        EncryptFunction(funcPtr, size, key);
    }

    bool SetMemoryExecutable(void* ptr, size_t size) {
        DWORD oldProtect;
        return VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &oldProtect) != 0;
    }
}

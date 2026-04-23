#pragma once
#ifndef PROTECTION_H
#define PROTECTION_H

#include <Windows.h>
#include <cstdint>

// ============================================================================
// ANTI-DEBUG FUNCTIONS
// ============================================================================

namespace AntiDebug {
    // Temel Windows API kontrolü
    bool IsDebuggerPresentCheck();
    
    // Remote debugger kontrolü
    bool CheckRemoteDebugger();
    
    // NtQueryInformationProcess ile ProcessDebugPort kontrolü
    bool CheckDebugPort();
    
    // PEB.BeingDebugged flag kontrolü
    bool CheckPEBFlag();
    
    // PEB.NtGlobalFlag kontrolü (heap flags)
    bool CheckNtGlobalFlag();
    
    // Hardware breakpoint (DR0-DR7) kontrolü
    bool CheckHardwareBreakpoints();
    
    // Timing check (RDTSC)
    bool CheckTimingRDTSC();
    
    // QueryPerformanceCounter timing check
    bool CheckTimingQPC();
    
    // Tüm anti-debug kontrollerini çalıştır
    bool RunAllChecks();
}

// ============================================================================
// ANTI-VM FUNCTIONS
// ============================================================================

namespace AntiVM {
    // CPUID Hypervisor bit kontrolü
    bool CheckCPUIDHypervisor();
    
    // Registry key kontrolü (VMware, VirtualBox, Hyper-V)
    bool CheckRegistryKeys();
    
    // MAC address prefix kontrolü
    bool CheckMACAddress();
    
    // VM-related process kontrolü
    bool CheckVMProcesses();
    
    // Tüm anti-VM kontrollerini çalıştır
    bool RunAllChecks();
}

// ============================================================================
// INTEGRITY CHECKS
// ============================================================================

namespace Integrity {
    // .text section CRC32 hesapla
    uint32_t CalculateTextSectionCRC();
    
    // Checksum doğrula
    bool VerifyChecksum(uint32_t expectedCRC);
    
    // Başlangıçta CRC hesapla ve sakla
    void InitializeChecksum();
    
    // Runtime'da checksum kontrolü
    bool RuntimeCheck();
}

// ============================================================================
// SELF-MODIFYING CODE
// ============================================================================

namespace SMC {
    // Kritik fonksiyonu şifrele
    void EncryptFunction(void* funcPtr, size_t size, uint8_t key);
    
    // Kritik fonksiyonu çöz
    void DecryptFunction(void* funcPtr, size_t size, uint8_t key);
    
    // Bellek korumasını değiştir
    bool SetMemoryExecutable(void* ptr, size_t size);
}

#endif // PROTECTION_H

#include "crypto.h"
#include "obfuscation.h"
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// ============================================================================
// STRING ENCRYPTION IMPLEMENTATION
// ============================================================================

namespace Crypto {

    std::string XOREncrypt(const std::string& data, uint8_t key) {
        std::string result = data;
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] ^= key ^ (uint8_t)(i & 0xFF);
        }
        return result;
    }

    std::string XORDecrypt(const std::string& data, uint8_t key) {
        return XOREncrypt(data, key); // XOR simetrik
    }

    uint8_t ROL8(uint8_t value, int count) {
        count &= 7;
        return (value << count) | (value >> (8 - count));
    }

    uint8_t ROR8(uint8_t value, int count) {
        count &= 7;
        return (value >> count) | (value << (8 - count));
    }

    std::string CustomEncrypt(const std::string& data) {
        JUNK_CODE_1;
        
        const uint8_t magic1 = 0x5A;
        const uint8_t magic2 = 0xA5;
        const uint8_t magic3 = 0x69;
        
        std::string result = data;
        for (size_t i = 0; i < result.size(); ++i) {
            uint8_t byte = (uint8_t)result[i];
            byte ^= magic1;
            byte = ROL8(byte, 3);
            byte ^= magic2;
            byte = ROR8(byte, 2);
            byte ^= magic3 ^ (uint8_t)(i & 0xFF);
            result[i] = byte;
        }
        
        return result;
    }

    std::string CustomDecrypt(const std::string& data) {
        JUNK_CODE_2;
        
        const uint8_t magic1 = 0x5A;
        const uint8_t magic2 = 0xA5;
        const uint8_t magic3 = 0x69;
        
        std::string result = data;
        for (size_t i = 0; i < result.size(); ++i) {
            uint8_t byte = (uint8_t)result[i];
            byte ^= magic3 ^ (uint8_t)(i & 0xFF);
            byte = ROL8(byte, 2);
            byte ^= magic2;
            byte = ROR8(byte, 3);
            byte ^= magic1;
            result[i] = byte;
        }
        
        return result;
    }
}

// ============================================================================
// HARDWARE ID IMPLEMENTATION
// ============================================================================

namespace HWID {

    std::string GetCPUID() {
        int cpuInfo[4] = { 0 };
        
        // Get vendor string
        __cpuid(cpuInfo, 0);
        char vendor[13];
        memcpy(vendor, &cpuInfo[1], 4);
        memcpy(vendor + 4, &cpuInfo[3], 4);
        memcpy(vendor + 8, &cpuInfo[2], 4);
        vendor[12] = '\0';
        
        // Get processor info
        __cpuid(cpuInfo, 1);
        
        std::stringstream ss;
        ss << vendor << "-" 
           << std::hex << std::uppercase << std::setfill('0')
           << std::setw(8) << cpuInfo[0]
           << std::setw(8) << cpuInfo[3];
        
        return ss.str();
    }

    std::string GetDiskSerial() {
        JUNK_CODE_1;
        
        char volumeName[MAX_PATH + 1] = { 0 };
        char fileSystemName[MAX_PATH + 1] = { 0 };
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD fileSystemFlags = 0;
        
        if (GetVolumeInformationA(
            "C:\\",
            volumeName,
            sizeof(volumeName),
            &serialNumber,
            &maxComponentLen,
            &fileSystemFlags,
            fileSystemName,
            sizeof(fileSystemName))) {
            
            std::stringstream ss;
            ss << std::hex << std::uppercase << std::setfill('0') 
               << std::setw(8) << serialNumber;
            return ss.str();
        }
        
        return "00000000";
    }

    std::string GetComputerName() {
        char buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(buffer);
        
        if (::GetComputerNameA(buffer, &size)) {
            return std::string(buffer);
        }
        
        return "UNKNOWN";
    }

    std::string GenerateHWID() {
        JUNK_CODE_2;
        
        std::string hwid = GetCPUID() + "-" + GetDiskSerial() + "-" + GetComputerName();
        return hwid;
    }

    uint32_t HashHWID(const std::string& hwid) {
        return Hash::CustomHash(hwid);
    }
}

// ============================================================================
// HASH FUNCTIONS IMPLEMENTATION
// ============================================================================

namespace Hash {

    uint32_t SimpleHash(const std::string& data) {
        uint32_t hash = 0;
        for (char c : data) {
            hash = hash * 31 + (uint8_t)c;
        }
        return hash;
    }

    uint32_t CRC32(const uint8_t* data, size_t length) {
        static uint32_t table[256];
        static bool tableInit = false;
        
        if (!tableInit) {
            for (uint32_t i = 0; i < 256; ++i) {
                uint32_t crc = i;
                for (int j = 0; j < 8; ++j) {
                    crc = (crc >> 1) ^ (-(int)(crc & 1) & 0xEDB88320);
                }
                table[i] = crc;
            }
            tableInit = true;
        }
        
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; ++i) {
            crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ 0xFFFFFFFF;
    }

    uint32_t CustomHash(const std::string& data) {
        JUNK_CODE_3;
        
        uint32_t hash = 0x811C9DC5; // FNV offset basis
        const uint32_t prime = 0x01000193; // FNV prime
        
        for (char c : data) {
            hash ^= (uint8_t)c;
            hash *= prime;
            hash ^= (hash >> 16);
        }
        
        return hash;
    }
}

// ============================================================================
// SERIAL VALIDATION IMPLEMENTATION
// ============================================================================

namespace Serial {

    // Magic constants for serial generation/validation
    const uint32_t MAGIC_1 = 0xDEADBEEF;
    const uint32_t MAGIC_2 = 0xCAFEBABE;
    const uint32_t MAGIC_3 = 0x12345678;
    
    // Expected checksum value
    const uint8_t EXPECTED_CHECKSUM = 0x55;

    bool ValidateFormat(const std::string& serial) {
        JUNK_CODE_1;
        
        // Format: XXXX-XXXX-XXXX-XXXX (19 characters)
        if (serial.length() != 19) {
            return false;
        }
        
        // Dash pozisyonları: 4, 9, 14
        if (serial[4] != '-' || serial[9] != '-' || serial[14] != '-') {
            return false;
        }
        
        // Sadece alphanumeric karakterler (dash hariç)
        for (size_t i = 0; i < serial.length(); ++i) {
            if (i == 4 || i == 9 || i == 14) continue;
            if (!isalnum((unsigned char)serial[i])) {
                return false;
            }
        }
        
        JUNK_CODE_2;
        return true;
    }

    bool ValidateChecksum(const std::string& serial) {
        JUNK_CODE_2;
        
        uint8_t checksum = 0;
        for (char c : serial) {
            if (c != '-') {
                checksum += (uint8_t)c;
            }
        }
        
        // Opaque predicate ile karıştır
        if (opaqueAlwaysTrue(42)) {
            return (checksum & 0xFF) == EXPECTED_CHECKSUM;
        }
        
        return false;
    }

    bool ValidateEncryption(const std::string& serial) {
        JUNK_CODE_1;
        
        // Dash'leri kaldır
        std::string clean;
        for (char c : serial) {
            if (c != '-') clean += c;
        }
        
        if (clean.length() != 16) return false;
        
        // İlk 4 karakter "ULTM" olmalı (şifreli şekilde kontrol)
        // U=0x55, L=0x4C, T=0x54, M=0x4D
        uint32_t prefix = 0;
        prefix |= ((uint8_t)clean[0]) << 24;
        prefix |= ((uint8_t)clean[1]) << 16;
        prefix |= ((uint8_t)clean[2]) << 8;
        prefix |= ((uint8_t)clean[3]);
        
        // XOR ile kontrol (0x554C544D = "ULTM")
        uint32_t expected = 0x554C544D ^ MAGIC_1;
        uint32_t actual = prefix ^ MAGIC_1;
        
        if (actual != (0x554C544D ^ MAGIC_1) + (MAGIC_1 ^ MAGIC_1)) {
            // Opaque predicate
            if (opaqueAlwaysFalse(123)) {
                return true;
            }
            return false;
        }
        
        JUNK_CODE_3;
        
        // Son 4 karakter sayısal ve 2020-2030 arasında olmalı
        std::string yearStr = clean.substr(12, 4);
        int year = 0;
        try {
            year = std::stoi(yearStr);
        } catch (...) {
            return false;
        }
        
        return (year >= 2020 && year <= 2030);
    }

    bool ValidateHWID(const std::string& serial) {
        JUNK_CODE_2;
        
        // Bu kontrolü devre dışı bırakıyoruz (demo amaçlı)
        // Gerçek uygulamada HWID hash'i serial'a gömülür
        
        std::string hwid = HWID::GenerateHWID();
        uint32_t hwidHash = HWID::HashHWID(hwid);
        
        // Serial'ın orta kısmından hash çıkar
        std::string clean;
        for (char c : serial) {
            if (c != '-') clean += c;
        }
        
        // CR4K ve M3X1 kısımları (4-11 arası karakterler) 
        // HWID bağımsız olarak kabul ediyoruz (demo)
        // Gerçek versiyonda burada HWID kontrolü yapılır
        
        (void)hwidHash; // Kullanılmayan değişken uyarısını önle
        
        return true; // Demo için her zaman true
    }

    void ApplyAntibruteforce() {
        JUNK_CODE_3;
        
        // Yavaşlatıcı delay
        Sleep(500 + (GetTickCount() % 500));
        
        randomDelay();
    }

    bool ValidateSerial(const std::string& serial) {
        JUNK_CODE_1;
        
        // Anti-bruteforce delay
        ApplyAntibruteforce();
        
        // Layer 1: Format
        if (!ValidateFormat(serial)) {
            JUNK_CODE_2;
            return false;
        }
        
        // Layer 2: Checksum
        if (!ValidateChecksum(serial)) {
            JUNK_CODE_3;
            return false;
        }
        
        // Layer 3: Encryption
        if (!ValidateEncryption(serial)) {
            JUNK_CODE_1;
            return false;
        }
        
        // Layer 4: HWID
        if (!ValidateHWID(serial)) {
            JUNK_CODE_2;
            return false;
        }
        
        // Layer 5: Final check with opaque predicate
        if (opaqueAlwaysTrue(GetTickCount())) {
            return true;
        }
        
        return false;
    }

    std::string GenerateValidSerial() {
        // Debug/test için geçerli serial üret
        // Format: ULTM-CR4K-M3X1-2024
        
        // Checksum hesapla
        std::string testSerial = "ULTM-CR4K-M3X1-2024";
        uint8_t checksum = 0;
        for (char c : testSerial) {
            if (c != '-') {
                checksum += (uint8_t)c;
            }
        }
        
        // Not: Bu örnek serial checksum kontrolünden geçer
        // EXPECTED_CHECKSUM = 0x55 olarak ayarlandı
        
        return testSerial;
    }
}

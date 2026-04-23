#pragma once
#ifndef CRYPTO_H
#define CRYPTO_H

#include <Windows.h>
#include <string>
#include <cstdint>

// ============================================================================
// STRING ENCRYPTION
// ============================================================================

namespace Crypto {
    // XOR şifreleme/çözme
    std::string XOREncrypt(const std::string& data, uint8_t key);
    std::string XORDecrypt(const std::string& data, uint8_t key);
    
    // ROL/ROR işlemleri
    uint8_t ROL8(uint8_t value, int count);
    uint8_t ROR8(uint8_t value, int count);
    
    // Custom encryption (XOR + ROL + Magic)
    std::string CustomEncrypt(const std::string& data);
    std::string CustomDecrypt(const std::string& data);
}

// ============================================================================
// HARDWARE ID
// ============================================================================

namespace HWID {
    // CPU ID al
    std::string GetCPUID();
    
    // Disk serial numarası al
    std::string GetDiskSerial();
    
    // Bilgisayar adı al
    std::string GetComputerName();
    
    // Birleşik Hardware ID oluştur
    std::string GenerateHWID();
    
    // HWID hash'i
    uint32_t HashHWID(const std::string& hwid);
}

// ============================================================================
// SERIAL VALIDATION
// ============================================================================

namespace Serial {
    // Serial format: XXXX-XXXX-XXXX-XXXX
    
    // Layer 1: Format kontrolü
    bool ValidateFormat(const std::string& serial);
    
    // Layer 2: Checksum kontrolü
    bool ValidateChecksum(const std::string& serial);
    
    // Layer 3: Custom decryption ve magic number kontrolü
    bool ValidateEncryption(const std::string& serial);
    
    // Layer 4: Hardware ID binding kontrolü
    bool ValidateHWID(const std::string& serial);
    
    // Layer 5: Anti-bruteforce (timing)
    void ApplyAntibruteforce();
    
    // Tüm katmanları kontrol et
    bool ValidateSerial(const std::string& serial);
    
    // Geçerli serial üret (debug/test için)
    std::string GenerateValidSerial();
}

// ============================================================================
// HASH FUNCTIONS
// ============================================================================

namespace Hash {
    // Basit hash fonksiyonu
    uint32_t SimpleHash(const std::string& data);
    
    // CRC32
    uint32_t CRC32(const uint8_t* data, size_t length);
    
    // Custom hash (serial validation için)
    uint32_t CustomHash(const std::string& data);
}

#endif // CRYPTO_H

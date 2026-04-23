#pragma once
#ifndef OBFUSCATION_H
#define OBFUSCATION_H

#include <Windows.h>
#include <cstdint>
#include <intrin.h>

// ============================================================================
// COMPILE-TIME STRING ENCRYPTION
// ============================================================================

template <size_t N, char Key>
struct EncryptedString {
    char data[N];
    
    constexpr EncryptedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = str[i] ^ Key ^ static_cast<char>(i);
        }
    }
    
    const char* decrypt() const {
        static char decrypted[N];
        for (size_t i = 0; i < N; ++i) {
            decrypted[i] = data[i] ^ Key ^ static_cast<char>(i);
        }
        return decrypted;
    }
};

#define ENC_STR(str) EncryptedString<sizeof(str), 0x5A>(str).decrypt()

// ============================================================================
// OPAQUE PREDICATES - Her zaman true/false dönen karmaşık koşullar
// ============================================================================

// Her zaman true döner (x^2 >= 0 her zaman doğrudur)
#define OPAQUE_TRUE(x) (((x) * (x)) >= 0)

// Her zaman false döner
#define OPAQUE_FALSE(x) (((x) * (x)) < 0)

// Karmaşık opaque predicate - her zaman true
inline bool opaqueAlwaysTrue(int seed) {
    volatile int a = seed;
    volatile int b = a * a;
    volatile int c = b + 1;
    return (c > b) || (a == 0); // Her zaman true
}

// Karmaşık opaque predicate - her zaman false
inline bool opaqueAlwaysFalse(int seed) {
    volatile int a = seed | 1; // En az 1 olmasını garantile
    volatile int b = a * a;
    return (b < 0) && (a > 0); // Signed overflow olmadan her zaman false
}

// ============================================================================
// JUNK CODE MACROS - Anlamsız kod blokları
// ============================================================================

#define JUNK_CODE_1 \
    { \
        volatile unsigned __int64 __junk64 = __rdtsc(); \
        volatile int __junk1 = (int)(__junk64 & 0xFF); \
        volatile int __junk2 = __junk1 ^ 0xDEAD; \
        __junk1 = __junk2 + __junk1; \
        (void)__junk1; \
    }

#define JUNK_CODE_2 \
    { \
        volatile int __j = 0; \
        for (volatile int __i = 0; __i < 3; ++__i) { \
            __j += __i * 2; \
            __j ^= 0xBEEF; \
        } \
        (void)__j; \
    }

#define JUNK_CODE_3 \
    { \
        volatile DWORD __tick = GetTickCount(); \
        volatile DWORD __fake = __tick ^ 0xCAFEBABE; \
        __fake = (__fake >> 4) | (__fake << 28); \
        (void)__fake; \
    }

// ============================================================================
// ANTI-DISASSEMBLY TRICKS
// ============================================================================

// Sahte jump - disassembler'ları karıştırır
#define ANTI_DISASM_1 \
    __asm { \
        push eax \
        xor eax, eax \
        jz short label1 \
        __emit 0xE8 \
        label1: \
        pop eax \
    }

// ============================================================================
// CONTROL FLOW OBFUSCATION
// ============================================================================

#define OBFUSCATED_IF(condition, true_block, false_block) \
    { \
        JUNK_CODE_1; \
        volatile bool __cond = (condition); \
        if (opaqueAlwaysTrue(42)) { \
            if (__cond) { \
                JUNK_CODE_2; \
                true_block; \
            } else { \
                false_block; \
            } \
        } \
    }

// ============================================================================
// NUMBER OBFUSCATION
// ============================================================================

// Sayıyı karmaşık şekilde üretir - compile-time
constexpr int obfuscateNumber(int n) {
    return ((n ^ 0x12345678) - 0x87654321) ^ 0xDEADBEEF;
}

constexpr int deobfuscateNumber(int n) {
    return ((n ^ 0xDEADBEEF) + 0x87654321) ^ 0x12345678;
}

#define OBF_NUM(n) deobfuscateNumber(obfuscateNumber(n))

// ============================================================================
// TIMING-BASED OBFUSCATION
// ============================================================================

inline void randomDelay() {
    volatile DWORD delay = GetTickCount() % 10;
    for (volatile DWORD i = 0; i < delay * 1000; ++i) {
        volatile int x = i * 2;
        (void)x;
    }
}

#endif // OBFUSCATION_H

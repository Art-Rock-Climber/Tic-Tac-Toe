#include <Windows.h>
#include <Windowsx.h>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>

// ���������� �������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SaveState(HWND hwnd);
void LoadState(HWND hwnd);
COLORREF GetRandomColor();
void CreateWinAPIMenu(HWND hwnd);
void ResetGridState(HWND hwnd);
void ResetColors(HWND hwnd);
void UpdateWindowTitle(HWND hwnd);
bool CheckWinner(HWND hwnd);

// ���������� ����������
int n = 3; // ������ ����� � ������� �� ���������
const int defaultWidth = 320; // ������� ���� �� ���������
const int defaultHeight = 240;
std::vector<std::vector<char>> gridState; // ��������� �����: 'O' ��� �������, 'X' ��� ���������, '_' ��� ������
const wchar_t* stateFileName = L"grid_state.txt"; // ���� ��� ���������� � �������� �������� � ������ ���� � ����� � ��������� �����
COLORREF backgroundColor = RGB(0, 0, 255); // ���� ���� (����� ��� �� ���������)
COLORREF gridColor = RGB(255, 0, 0); // ���� ����� (������� ���� �� ���������)
char currentPlayer = 'X'; // ������� ����� ('X' �� ���������)
#define OnClickResetGrid 1
#define OnClickExit 2
#define OnClickResetColors 3

/// <summary>
/// ����� ����� � ����������
/// </summary>
/// <param name="hInst"> ���������� ���������� </param>
/// <param name="hPrevInstance"></param>
/// <param name="pCmdLine"> ��������� ��������� ������, ���������� ��� ������� ��������� </param>
/// <param name="nCmdShow"> ��������� ��������� ���� </param>
/// <returns></returns>
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // �������� � ����������� ������ ����
    WNDCLASS SoftwareWindowsClass = { 0 };
    SoftwareWindowsClass.hIcon = LoadIcon(NULL, IDI_HAND);
    SoftwareWindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    SoftwareWindowsClass.hInstance = hInst;
    SoftwareWindowsClass.lpszClassName = L"MainWinAPIClass";
    SoftwareWindowsClass.hbrBackground = CreateSolidBrush(backgroundColor);
    SoftwareWindowsClass.lpfnWndProc = WindowProc;

    if (!RegisterClassW(&SoftwareWindowsClass)) {
        return -1;
    }

    // �������� ����
    HWND hwnd = CreateWindowW(
        L"MainWinAPIClass",
        L"��������-������",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        defaultWidth,
        defaultHeight,
        NULL,
        NULL,
        hInst,
        NULL
    );

    // ��������� ��������� ��������� ������ ��� ��������� ������� �����
    if (pCmdLine && *pCmdLine) {
        n = _wtoi(pCmdLine);
        if (n <= 0) n = 3; // ������� � �������� �� ��������� ��� ������������ �����
    }

    // ������������� ��������� �����
    gridState.resize(n, std::vector<char>(n, '_'));
    LoadState(hwnd);

    CreateWinAPIMenu(hwnd);
    UpdateWindowTitle(hwnd);

    // ��������� ��������� ����
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

/// <summary>
/// ���������� ��������� ����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
/// <param name="uMsg"> ��� ��������� </param>
/// <param name="wParam"> �������������� ����������, ��������� � ���������� </param>
/// <param name="lParam"> �������������� ����������, ��������� � ���������� </param>
/// <returns></returns>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static float cellWidth, cellHeight;
    switch (uMsg) {
    case WM_CREATE:
    {
        // �������� ����
        CreateWinAPIMenu(hwnd);

        RECT rect;
        GetClientRect(hwnd, &rect);
        cellWidth = (static_cast<float>(rect.right) - rect.left) / n;
        cellHeight = (static_cast<float>(rect.bottom) - rect.top) / n;
        return 0;
    }
    case WM_SIZE:
    {
        // ��������� ������� ����
        RECT rect;
        GetClientRect(hwnd, &rect);
        cellWidth = (static_cast<float>(rect.right) - rect.left) / n;
        cellHeight = (static_cast<float>(rect.bottom) - rect.top) / n;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // ��������� �����
        HPEN hPen = CreatePen(PS_SOLID, 2, gridColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        for (int i = 1; i < n; ++i) {
            MoveToEx(hdc, i * cellWidth, 0, NULL);
            LineTo(hdc, i * cellWidth, n * cellHeight);

            MoveToEx(hdc, 0, i * cellHeight, NULL);
            LineTo(hdc, n * cellWidth, i * cellHeight);
        }

        // ��������� ��������� � �������
        for (int row = 0; row < n; ++row) {
            for (int col = 0; col < n; ++col) {
                int x = col * cellWidth;
                int y = row * cellHeight;

                if (gridState[row][col] == 'O') {
                    HPEN circlePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // ���������� �������

                    SelectObject(hdc, circlePen);
                    SelectObject(hdc, hBrush);
                    Ellipse(hdc, x + 0.9f * cellWidth, y + 0.9f * cellHeight, 
                        x + 0.1f * cellWidth, y + 0.1f * cellHeight);

                    DeleteObject(circlePen);
                }
                else if (gridState[row][col] == 'X') {
                    HPEN crossPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                    SelectObject(hdc, crossPen);

                    MoveToEx(hdc, x + 0.9f * cellWidth, y + 0.9f * cellHeight, NULL);
                    LineTo(hdc, x + 0.1f * cellWidth, y + 0.1f * cellHeight);
                    MoveToEx(hdc, x + 0.1f * cellWidth, y + 0.9f * cellHeight, NULL);
                    LineTo(hdc, x + 0.9f * cellWidth, y + 0.1f * cellHeight);

                    DeleteObject(crossPen);
                }
            }
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        // ���������� ��������� ��������� � �������
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        int col = xPos / cellWidth;
        int row = yPos / cellHeight;

        if (row < n && col < n && gridState[row][col] == '_') {
            if ((uMsg == WM_LBUTTONDOWN && currentPlayer == 'O') || (uMsg == WM_RBUTTONDOWN && currentPlayer == 'X')) {
                gridState[row][col] = currentPlayer;
                currentPlayer = (currentPlayer == 'O') ? 'X' : 'O';
            }
            if (CheckWinner(hwnd)) {
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            UpdateWindowTitle(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_KEYDOWN:
    {
        if ((wParam == 'Q' && (GetKeyState(VK_CONTROL) & 0x8000)) || wParam == VK_ESCAPE) {
            PostMessage(hwnd, WM_CLOSE, 0, 0); // �������� ����������
        }
        else if ((GetKeyState(VK_SHIFT) & 0x8000) && wParam == 'C') {
            ShellExecute(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL); // �������� ��������
        }
        else if (wParam == VK_RETURN) {
            // ����� ����� ���� �� ���������
            backgroundColor = GetRandomColor();
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(backgroundColor));
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if ((wParam == 'R' && (GetKeyState(VK_CONTROL) & 0x8000))) {
            ResetGridState(hwnd); // ���������� ������
        }
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        // ������� ��������� �����
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int r = GetRValue(gridColor);
        int g = GetGValue(gridColor);
        int b = GetBValue(gridColor);

        r = (r + delta / 6 + 256) % 256;
        g = (g + delta / 12 + 256) % 256;
        b = (b + delta / 4 + 256) % 256;

        gridColor = RGB(r, g, b);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_COMMAND:
        switch (wParam) {
        case OnClickResetGrid:
            ResetGridState(hwnd);
            break;
        case OnClickResetColors:
            ResetColors(hwnd);
            break;
        case OnClickExit:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
        return 0;
    case WM_DESTROY:
        SaveState(hwnd);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

/// <summary>
/// ������� ��� ���������� ��������� ����� � ����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void SaveState(HWND hwnd) {
    std::ofstream outFile("grid_state.txt");
    if (outFile.is_open()) {
        // ������ ����� � �������
        outFile << n << std::endl;

        // ������ ����
        RECT rect;
        GetWindowRect(hwnd, &rect);
        outFile << rect.right - rect.left << " " << rect.bottom - rect.top << std::endl;

        // ���� ���� � �����
        outFile << static_cast<int>(GetRValue(backgroundColor)) << " "
            << static_cast<int>(GetGValue(backgroundColor)) << " "
            << static_cast<int>(GetBValue(backgroundColor)) << std::endl;
        outFile << static_cast<int>(GetRValue(gridColor)) << " "
            << static_cast<int>(GetGValue(gridColor)) << " "
            << static_cast<int>(GetBValue(gridColor)) << std::endl;

        // ������� �����
        outFile << currentPlayer << std::endl;

        // ������� �����
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                outFile << gridState[i][j];
            }
            outFile << std::endl;
        }

        outFile.close();
    }
}

/// <summary>
/// ������� ��� �������� ��������� ����� �� �����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void LoadState(HWND hwnd) {
    std::ifstream inFile("grid_state.txt");
    if (inFile.is_open()) {
        // ������ ����� � �������
        int savedN;
        inFile >> savedN;
        inFile.ignore(); // ���������� ������ ����� ������ ����� �����

        // ������ ����
        int width, height;
        inFile >> width >> height;
        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        inFile.ignore();

        // ���� ���� � �����
        int r, g, b;
        inFile >> r >> g >> b;
        backgroundColor = RGB(r, g, b);
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(backgroundColor));
        inFile >> r >> g >> b;
        gridColor = RGB(r, g, b);
        inFile.ignore();

        // ������� �����
        inFile >> currentPlayer;
        inFile.ignore();

        // ��������� �����
        if (savedN == n) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    inFile.get(gridState[i][j]);
                }
                inFile.ignore();
            }
        }

        inFile.close();
    }
}

/// <summary>
/// ������� ��� ��������� ���������� �����
/// </summary>
/// <returns> ���� � ������� RGB(r, g, b) </returns>
COLORREF GetRandomColor() {
    srand((unsigned)(time(0)));
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

/// <summary>
/// �������� ����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void CreateWinAPIMenu(HWND hwnd) {
    HMENU rootMenu = CreateMenu();
    HMENU subMenu = CreateMenu();
    AppendMenu(rootMenu, MF_POPUP, (int)subMenu, L"����");
    AppendMenu(subMenu, MF_STRING, OnClickResetGrid, L"����� ��������� �����");
    AppendMenu(subMenu, MF_STRING, OnClickResetColors, L"����� ������");
    AppendMenu(subMenu, MF_STRING, OnClickExit, L"�����");

    SetMenu(hwnd, rootMenu);
}

/// <summary>
/// ����� ��������� �����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void ResetGridState(HWND hwnd) {
    for (auto& row : gridState) {
        std::fill(row.begin(), row.end(), '_');
    }
    currentPlayer = 'X';
    UpdateWindowTitle(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

/// <summary>
/// ����� ������
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void ResetColors(HWND hwnd) {
    backgroundColor = RGB(0, 0, 255);
    gridColor = RGB(255, 0, 0);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(backgroundColor));
    InvalidateRect(hwnd, NULL, TRUE);
}

/// <summary>
/// ���������� �������� ����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
void UpdateWindowTitle(HWND hwnd) {
    wchar_t title[50];
    wsprintf(title, L"��������-������ - ��� ������: %c", currentPlayer);
    SetWindowText(hwnd, title);
}

/// <summary>
/// �������� �� ���������� ����
/// </summary>
/// <param name="hwnd"> ���������� ���� </param>
/// <returns></returns>
bool CheckWinner(HWND hwnd) {
    char winner = '_';
    // �������� ������������ � ����������
    for (int i = 0; i < n; ++i) {
        if (winner == '_' && gridState[i][0] != '_' && std::all_of(gridState[i].begin(), gridState[i].end(), [i](char c) { return c == gridState[i][0]; }))
            winner = gridState[i][0];
        if (winner == '_' && gridState[0][i] != '_' && std::all_of(gridState.begin(), gridState.end(), [i](std::vector<char>& row) { return row[i] == gridState[0][i]; }))
            winner = gridState[0][i];
    }

    // �������� ����������
    if (winner == '_') {
        bool left_diag = true, right_diag = true;
        for (int i = 0; i < n; ++i) {
            if (gridState[i][i] != gridState[0][0]) left_diag = false;
            if (gridState[i][n - 1 - i] != gridState[0][n - 1]) right_diag = false;
        }

        if (gridState[0][0] != '_' && left_diag) winner = gridState[0][0];
        if (gridState[0][n - 1] != '_' && right_diag) winner = gridState[0][n - 1];
    }

    // ���������� ����
    if (winner == '_') return false;
    else {
        MessageBox(hwnd, (std::wstring(L"����� ") + static_cast<wchar_t>(winner) + L" �������!").c_str(), L"������", MB_OK);
        ResetGridState(hwnd);
        return true;
    }
}

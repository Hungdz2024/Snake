#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

// Hàm chuyển số nguyên sang wstring tương thích mọi bản MinGW
wstring intToWString(int val) {
    wstringstream wss;
    wss << val;
    return wss.str();
}

// ==========================================
// CẤU HÌNH VÀ THÔNG SỐ GAME
// ==========================================
const int CELL_SIZE = 24;          // Kích thước 1 ô cờ (pixel)
const int GRID_WIDTH = 25;         // Số cột
const int GRID_HEIGHT = 20;        // Số hàng
const int SIDEBAR_WIDTH = 220;     // Chiều rộng bảng thông tin bên phải
const int WINDOW_WIDTH = (GRID_WIDTH * CELL_SIZE) + SIDEBAR_WIDTH + 16;
const int WINDOW_HEIGHT = (GRID_HEIGHT * CELL_SIZE) + 39;

const int TIMER_ID = 1;
const string RECORD_FILE = "highscores.txt";

// Các trạng thái của game
enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

// Hướng di chuyển
enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

struct Point {
    int x;
    int y;
};

// ==========================================
// BIẾN TOÀN CỤC CỦA TRÒ CHƠI
// ==========================================
GameState g_state = STATE_MENU;
bool g_passWallMode = false;
Direction g_dir = DIR_RIGHT;
Direction g_nextDir = DIR_RIGHT;

vector<Point> g_snake;
Point g_food;
Point g_specialFood;
bool g_hasSpecialFood = false;
int g_specialTimer = 0;

int g_score = 0;
int g_highScore = 0;
int g_speedMs = 120;

// ==========================================
// QUẢN LÝ ĐIỂM SỐ KỶ LỤC
// ==========================================
void loadHighScore() {
    ifstream fin(RECORD_FILE);
    if (fin.is_open()) {
        fin >> g_highScore;
        fin.close();
    }
}

void saveHighScore() {
    if (g_score > g_highScore) {
        g_highScore = g_score;
        ofstream fout(RECORD_FILE);
        if (fout.is_open()) {
            fout << g_highScore;
            fout.close();
        }
    }
}

// ==========================================
// CƠ CHẾ SINH MỒI
// ==========================================
void spawnNormalFood() {
    bool onSnake;
    do {
        onSnake = false;
        g_food.x = rand() % GRID_WIDTH;
        g_food.y = rand() % GRID_HEIGHT;
        for (size_t i = 0; i < g_snake.size(); ++i) {
            if (g_snake[i].x == g_food.x && g_snake[i].y == g_food.y) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);
}

void spawnSpecialFood() {
    bool invalid;
    do {
        invalid = false;
        g_specialFood.x = rand() % GRID_WIDTH;
        g_specialFood.y = rand() % GRID_HEIGHT;
        if (g_specialFood.x == g_food.x && g_specialFood.y == g_food.y) invalid = true;
        for (size_t i = 0; i < g_snake.size(); ++i) {
            if (g_snake[i].x == g_specialFood.x && g_snake[i].y == g_specialFood.y) {
                invalid = true;
                break;
            }
        }
    } while (invalid);

    g_hasSpecialFood = true;
    g_specialTimer = 50; // Khoảng 5-6 giây
}

// ==========================================
// KHỞI TẠO VÁN CHƠI MỚI
// ==========================================
void initGame(bool passWall) {
    g_passWallMode = passWall;
    g_snake.clear();

    int startX = GRID_WIDTH / 2;
    int startY = GRID_HEIGHT / 2;
    Point p1 = {startX, startY};
    Point p2 = {startX - 1, startY};
    Point p3 = {startX - 2, startY};
    g_snake.push_back(p1);
    g_snake.push_back(p2);
    g_snake.push_back(p3);

    g_dir = DIR_RIGHT;
    g_nextDir = DIR_RIGHT;
    g_score = 0;
    g_speedMs = 120;
    g_hasSpecialFood = false;

    spawnNormalFood();
    g_state = STATE_PLAYING;
}

// ==========================================
// CẬP NHẬT LOGIC GAME MỖI FRAME
// ==========================================
void updateGame(HWND hwnd) {
    if (g_state != STATE_PLAYING) return;

    g_dir = g_nextDir;

    // Giảm thời gian mồi đặc biệt
    if (g_hasSpecialFood) {
        g_specialTimer--;
        if (g_specialTimer <= 0) {
            g_hasSpecialFood = false;
        }
    } else {
        if (rand() % 80 == 0) {
            spawnSpecialFood();
        }
    }

    // Tính toạ độ mới cho đầu rắn
    Point head = g_snake.front();
    switch (g_dir) {
        case DIR_UP:    head.y--; break;
        case DIR_DOWN:  head.y++; break;
        case DIR_LEFT:  head.x--; break;
        case DIR_RIGHT: head.x++; break;
    }

    // Xử lý va chạm biên tường
    if (g_passWallMode) {
        if (head.x < 0) head.x = GRID_WIDTH - 1;
        else if (head.x >= GRID_WIDTH) head.x = 0;
        if (head.y < 0) head.y = GRID_HEIGHT - 1;
        else if (head.y >= GRID_HEIGHT) head.y = 0;
    } else {
        if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) {
            saveHighScore();
            g_state = STATE_GAMEOVER;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }

    // Kiểm tra cắn vào thân rắn
    for (size_t i = 0; i < g_snake.size() - 1; ++i) {
        if (head.x == g_snake[i].x && head.y == g_snake[i].y) {
            saveHighScore();
            g_state = STATE_GAMEOVER;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }

    // Kiểm tra ăn mồi
    bool ateNormal = (head.x == g_food.x && head.y == g_food.y);
    bool ateSpecial = (g_hasSpecialFood && head.x == g_specialFood.x && head.y == g_specialFood.y);

    g_snake.insert(g_snake.begin(), head);

    if (ateNormal) {
        g_score += 10;
        spawnNormalFood();
        if (g_speedMs > 45) {
            g_speedMs -= 3;
            SetTimer(hwnd, TIMER_ID, g_speedMs, NULL);
        }
    } else if (ateSpecial) {
        g_score += 30;
        g_hasSpecialFood = false;
    } else {
        g_snake.pop_back();
    }

    InvalidateRect(hwnd, NULL, FALSE);
}

// ==========================================
// VẼ GIAO DIỆN GUI BẰNG GDI (CHỐNG GIẬT LAG)
// ==========================================
void render(HWND hwnd, HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right;
    int height = clientRect.bottom;

    // Kỹ thuật Double Buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 1. Tô nền bàn cờ
    HBRUSH bgBrush = CreateSolidBrush(RGB(22, 27, 34));
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    int playAreaWidth = GRID_WIDTH * CELL_SIZE;
    int playAreaHeight = GRID_HEIGHT * CELL_SIZE;

    // 2. Vẽ lưới ô cờ nhẹ
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(30, 36, 46));
    HPEN oldPen = (HPEN)SelectObject(memDC, gridPen);
    for (int x = 0; x <= GRID_WIDTH; ++x) {
        MoveToEx(memDC, x * CELL_SIZE, 0, NULL);
        LineTo(memDC, x * CELL_SIZE, playAreaHeight);
    }
    for (int y = 0; y <= GRID_HEIGHT; ++y) {
        MoveToEx(memDC, 0, y * CELL_SIZE, NULL);
        LineTo(memDC, playAreaWidth, y * CELL_SIZE);
    }
    SelectObject(memDC, oldPen);
    DeleteObject(gridPen);

    // 3. Đường viền ngăn cách sân chơi và Sidebar
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(48, 54, 61));
    SelectObject(memDC, borderPen);
    MoveToEx(memDC, playAreaWidth, 0, NULL);
    LineTo(memDC, playAreaWidth, playAreaHeight);
    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);

    // 4. Vẽ mồi thường
    HBRUSH foodBrush = CreateSolidBrush(RGB(248, 81, 73));
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, foodBrush);
    Ellipse(memDC, g_food.x * CELL_SIZE + 2, g_food.y * CELL_SIZE + 2, 
            (g_food.x + 1) * CELL_SIZE - 2, (g_food.y + 1) * CELL_SIZE - 2);
    DeleteObject(foodBrush);

    // 5. Vẽ mồi đặc biệt
    if (g_hasSpecialFood) {
        HBRUSH specBrush = CreateSolidBrush(RGB(240, 198, 60));
        SelectObject(memDC, specBrush);
        Ellipse(memDC, g_specialFood.x * CELL_SIZE + 1, g_specialFood.y * CELL_SIZE + 1,
                (g_specialFood.x + 1) * CELL_SIZE - 1, (g_specialFood.y + 1) * CELL_SIZE - 1);
        DeleteObject(specBrush);
    }

    // 6. Vẽ thân và đầu rắn
    HBRUSH bodyBrush = CreateSolidBrush(RGB(46, 160, 67));
    HBRUSH headBrush = CreateSolidBrush(RGB(86, 211, 100));

    for (size_t i = 0; i < g_snake.size(); ++i) {
        int x1 = g_snake[i].x * CELL_SIZE + 1;
        int y1 = g_snake[i].y * CELL_SIZE + 1;
        int x2 = (g_snake[i].x + 1) * CELL_SIZE - 1;
        int y2 = (g_snake[i].y + 1) * CELL_SIZE - 1;

        if (i == 0) {
            SelectObject(memDC, headBrush);
            RoundRect(memDC, x1, y1, x2, y2, 8, 8);
        } else {
            SelectObject(memDC, bodyBrush);
            RoundRect(memDC, x1, y1, x2, y2, 6, 6);
        }
    }
    DeleteObject(bodyBrush);
    DeleteObject(headBrush);

    // 7. Vẽ thanh Sidebar thông tin
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(230, 237, 243));

    HFONT hFontTitle = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hFontNormal = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                   DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT hOldFont = (HFONT)SelectObject(memDC, hFontTitle);

    int sideX = playAreaWidth + 20;
    TextOut(memDC, sideX, 20, L"SNAKE GAME", 10);

    SelectObject(memDC, hFontNormal);
    SetTextColor(memDC, RGB(139, 148, 158));
    TextOut(memDC, sideX, 60, L"Diem so:", 8);

    SetTextColor(memDC, RGB(86, 211, 100));
    wstring scoreStr = intToWString(g_score);
    TextOut(memDC, sideX + 80, 60, scoreStr.c_str(), (int)scoreStr.length());

    SetTextColor(memDC, RGB(139, 148, 158));
    TextOut(memDC, sideX, 90, L"Ky luc:", 7);
    SetTextColor(memDC, RGB(240, 198, 60));
    wstring highStr = intToWString(g_highScore);
    TextOut(memDC, sideX + 80, 90, highStr.c_str(), (int)highStr.length());

    SetTextColor(memDC, RGB(139, 148, 158));
    TextOut(memDC, sideX, 125, L"Che do tuong:", 13);
    if (g_passWallMode) {
        SetTextColor(memDC, RGB(86, 211, 100));
        TextOut(memDC, sideX, 148, L"[Xuyen tuong]", 13);
    } else {
        SetTextColor(memDC, RGB(248, 81, 73));
        TextOut(memDC, sideX, 148, L"[Dam la thua]", 13);
    }

    SetTextColor(memDC, RGB(139, 148, 158));
    TextOut(memDC, sideX, 185, L"Moi dac biet:", 13);
    if (g_hasSpecialFood) {
        SetTextColor(memDC, RGB(240, 198, 60));
        wstring timerStr = L"Co! (" + intToWString(g_specialTimer / 10 + 1) + L"s)";
        TextOut(memDC, sideX, 208, timerStr.c_str(), (int)timerStr.length());
    } else {
        SetTextColor(memDC, RGB(110, 118, 129));
        TextOut(memDC, sideX, 208, L"Chua co", 7);
    }

    SetTextColor(memDC, RGB(110, 118, 129));
    TextOut(memDC, sideX, 260, L"--- DIEU KHIEN ---", 18);
    TextOut(memDC, sideX, 290, L"W / A / S / D hoac", 18);
    TextOut(memDC, sideX, 310, L"Phim mui ten", 12);
    TextOut(memDC, sideX, 340, L"P: Tam dung", 11);
    TextOut(memDC, sideX, 370, L"ESC: Ve menu", 12);

    // 8. Vẽ lớp phủ Overlay
    if (g_state == STATE_MENU) {
        HBRUSH modalBrush = CreateSolidBrush(RGB(13, 17, 23));
        RECT modalRect = { 80, 80, playAreaWidth - 80, playAreaHeight - 80 };
        FillRect(memDC, &modalRect, modalBrush);
        DeleteObject(modalBrush);

        SelectObject(memDC, hFontTitle);
        SetTextColor(memDC, RGB(86, 211, 100));
        TextOut(memDC, 180, 120, L"RAN SAN MOI C++", 15);

        SelectObject(memDC, hFontNormal);
        SetTextColor(memDC, RGB(230, 237, 243));
        TextOut(memDC, 140, 180, L"Nhan [1] : Choi che do Co dien", 30);
        TextOut(memDC, 140, 220, L"Nhan [2] : Choi che do Xuyen tuong", 34);
        SetTextColor(memDC, RGB(139, 148, 158));
        TextOut(memDC, 140, 270, L"Nhan [ESC]: Thoat game", 22);
    } else if (g_state == STATE_PAUSED) {
        HBRUSH modalBrush = CreateSolidBrush(RGB(13, 17, 23));
        RECT modalRect = { 150, 160, playAreaWidth - 150, playAreaHeight - 160 };
        FillRect(memDC, &modalRect, modalBrush);
        DeleteObject(modalBrush);

        SelectObject(memDC, hFontTitle);
        SetTextColor(memDC, RGB(240, 198, 60));
        TextOut(memDC, 210, 190, L"TAM DUNG", 8);
        SelectObject(memDC, hFontNormal);
        SetTextColor(memDC, RGB(230, 237, 243));
        TextOut(memDC, 195, 230, L"Nhan [P] de tiep tuc", 20);
    } else if (g_state == STATE_GAMEOVER) {
        HBRUSH modalBrush = CreateSolidBrush(RGB(13, 17, 23));
        RECT modalRect = { 130, 130, playAreaWidth - 130, playAreaHeight - 130 };
        FillRect(memDC, &modalRect, modalBrush);
        DeleteObject(modalBrush);

        SelectObject(memDC, hFontTitle);
        SetTextColor(memDC, RGB(248, 81, 73));
        TextOut(memDC, 215, 160, L"GAME OVER!", 10);

        SelectObject(memDC, hFontNormal);
        SetTextColor(memDC, RGB(230, 237, 243));
        wstring finalScoreStr = L"Diem cua ban: " + intToWString(g_score);
        TextOut(memDC, 210, 205, finalScoreStr.c_str(), (int)finalScoreStr.length());

        SetTextColor(memDC, RGB(86, 211, 100));
        TextOut(memDC, 165, 250, L"Nhan [SPACE] de choi lai", 24);
        SetTextColor(memDC, RGB(139, 148, 158));
        TextOut(memDC, 175, 280, L"Nhan [ESC] de ve Menu", 21);
    }

    SelectObject(memDC, hOldFont);
    DeleteObject(hFontTitle);
    DeleteObject(hFontNormal);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// ==========================================
// HÀM XỬ LÝ SỰ KIỆN CỬA SỔ (WNDPROC)
// ==========================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            loadHighScore();
            SetTimer(hwnd, TIMER_ID, g_speedMs, NULL);
            break;

        case WM_TIMER:
            if (wParam == TIMER_ID) {
                updateGame(hwnd);
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            render(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_KEYDOWN:
            if (g_state == STATE_MENU) {
                if (wParam == '1' || wParam == VK_NUMPAD1) {
                    initGame(false);
                    SetTimer(hwnd, TIMER_ID, g_speedMs, NULL);
                } else if (wParam == '2' || wParam == VK_NUMPAD2) {
                    initGame(true);
                    SetTimer(hwnd, TIMER_ID, g_speedMs, NULL);
                } else if (wParam == VK_ESCAPE) {
                    PostQuitMessage(0);
                }
            } else if (g_state == STATE_PLAYING) {
                switch (wParam) {
                    case VK_UP:
                    case 'W':
                        if (g_dir != DIR_DOWN) g_nextDir = DIR_UP;
                        break;
                    case VK_DOWN:
                    case 'S':
                        if (g_dir != DIR_UP) g_nextDir = DIR_DOWN;
                        break;
                    case VK_LEFT:
                    case 'A':
                        if (g_dir != DIR_RIGHT) g_nextDir = DIR_LEFT;
                        break;
                    case VK_RIGHT:
                    case 'D':
                        if (g_dir != DIR_LEFT) g_nextDir = DIR_RIGHT;
                        break;
                    case 'P':
                        g_state = STATE_PAUSED;
                        InvalidateRect(hwnd, NULL, FALSE);
                        break;
                    case VK_ESCAPE:
                        g_state = STATE_MENU;
                        InvalidateRect(hwnd, NULL, FALSE);
                        break;
                }
            } else if (g_state == STATE_PAUSED) {
                if (wParam == 'P' || wParam == VK_SPACE) {
                    g_state = STATE_PLAYING;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (wParam == VK_ESCAPE) {
                    g_state = STATE_MENU;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (g_state == STATE_GAMEOVER) {
                if (wParam == VK_SPACE) {
                    initGame(g_passWallMode);
                    SetTimer(hwnd, TIMER_ID, g_speedMs, NULL);
                } else if (wParam == VK_ESCAPE) {
                    g_state = STATE_MENU;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;

        case WM_DESTROY:
            saveHighScore();
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ==========================================
// HÀM WINMAIN: ĐIỂM BẮT ĐẦU CHƯƠNG TRÌNH GUI
// ==========================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL));

    const wchar_t CLASS_NAME[] = L"SnakeGameWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Trò Chơi Rắn Săn Mồi - GUI Edition",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
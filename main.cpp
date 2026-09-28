#include <iostream>
#include <windows.h>
#include <conio.h>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <limits>

using namespace std;

// ==========================================
// CẤU HÌNH VÀ HẰNG SỐ CỦA GAME
// ==========================================
const int BOARD_WIDTH = 50;       // Chiều rộng khung sân chơi
const int BOARD_HEIGHT = 22;      // Chiều cao khung sân chơi
const int OFFSET_X = 5;           // Lề trái console
const int OFFSET_Y = 2;           // Lề trên console
const int MAX_SNAKE_LEN = 1000;   // Chiều dài tối đa của rắn
const string RECORD_FILE = "highscores.txt";

// Bảng mã màu console chuẩn Windows (SetConsoleTextAttribute)
enum ConsoleColor {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHTGRAY = 7,
    COLOR_DARKGRAY = 8,
    COLOR_LIGHTBLUE = 9,
    COLOR_LIGHTGREEN = 10,
    COLOR_LIGHTCYAN = 11,
    COLOR_LIGHTRED = 12,
    COLOR_LIGHTMAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
};

// Hướng di chuyển của rắn
enum Direction {
    DIR_UP = 0,
    DIR_DOWN = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
};

// Cấu trúc lưu kỷ lục điểm số
struct HighScore {
    string playerName;
    int score;
};

// ==========================================
// CÁC HÀM TIỆN ÍCH CONSOLE (WINDOWS API)
// ==========================================

// Đặt vị trí con trỏ màn hình không gây nhấp nháy
void goToXY(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Ẩn/Hiện con trỏ nhấp nháy trên console
void setCursorVisible(bool visible) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// Đổi màu chữ và màu nền hiển thị
void setColor(int textColor, int bgColor = COLOR_BLACK) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((bgColor << 4) | textColor));
}

// ==========================================
// QUẢN LÝ TỆP TIN VÀ BẢNG XẾP HẠNG
// ==========================================

// Đọc danh sách điểm kỷ lục từ file
vector<HighScore> loadHighScores() {
    vector<HighScore> list;
    ifstream fin(RECORD_FILE);
    if (!fin.is_open()) return list;

    HighScore item;
    while (fin >> item.playerName >> item.score) {
        list.push_back(item);
    }
    fin.close();

    sort(list.begin(), list.end(), [](const HighScore &a, const HighScore &b) {
        return a.score > b.score;
    });
    return list;
}

// Lưu điểm số mới vào file kỷ lục (lấy Top 5)
void saveHighScore(const string &name, int score) {
    vector<HighScore> list = loadHighScores();
    list.push_back({name, score});
    sort(list.begin(), list.end(), [](const HighScore &a, const HighScore &b) {
        return a.score > b.score;
    });

    if (list.size() > 5) {
        list.resize(5);
    }

    ofstream fout(RECORD_FILE);
    if (fout.is_open()) {
        for (const auto &item : list) {
            fout << item.playerName << " " << item.score << "\n";
        }
        fout.close();
    }
}

// ==========================================
// HÀM VẼ GIAO DIỆN & KHUNG CHƠI
// ==========================================

// Vẽ khung viền sân chơi
void drawGameBoard() {
    setColor(COLOR_CYAN);
    // Khung viền trên
    goToXY(OFFSET_X, OFFSET_Y);
    cout << "+";
    for (int i = 0; i < BOARD_WIDTH; ++i) cout << "-";
    cout << "+";

    // Khung viền hai bên
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        goToXY(OFFSET_X, OFFSET_Y + 1 + y);
        cout << "|";
        goToXY(OFFSET_X + BOARD_WIDTH + 1, OFFSET_Y + 1 + y);
        cout << "|";
    }

    // Khung viền dưới
    goToXY(OFFSET_X, OFFSET_Y + BOARD_HEIGHT + 1);
    cout << "+";
    for (int i = 0; i < BOARD_WIDTH; ++i) cout << "-";
    cout << "+";
    setColor(COLOR_WHITE);
}

// Vẽ bảng thông số trận đấu bên cánh phải
void drawGameInfo(int score, int speedMs, bool passWallMode, bool hasSpecialFood, int specialFoodTimer) {
    int infoX = OFFSET_X + BOARD_WIDTH + 5;
    int infoY = OFFSET_Y;

    setColor(COLOR_YELLOW);
    goToXY(infoX, infoY);     cout << "=============================";
    goToXY(infoX, infoY + 1); cout << "       THONG TIN MAN CHOI    ";
    goToXY(infoX, infoY + 2); cout << "=============================";

    setColor(COLOR_WHITE);
    goToXY(infoX, infoY + 4); cout << "Diem hien tai : ";
    setColor(COLOR_LIGHTGREEN);
    cout << score << "   ";

    setColor(COLOR_WHITE);
    goToXY(infoX, infoY + 5); cout << "Toc do delay  : ";
    setColor(COLOR_LIGHTCYAN);
    cout << speedMs << " ms  ";

    setColor(COLOR_WHITE);
    goToXY(infoX, infoY + 6); cout << "Che do tuong  : ";
    if (passWallMode) {
        setColor(COLOR_LIGHTGREEN);
        cout << "[XUYEN TUONG]       ";
    } else {
        setColor(COLOR_LIGHTRED);
        cout << "[DAP TUONG LA CHET] ";
    }

    setColor(COLOR_WHITE);
    goToXY(infoX, infoY + 8); cout << "Moi dac biet  : ";
    if (hasSpecialFood) {
        setColor(COLOR_LIGHTMAGENTA);
        cout << "Dang xuat hien (" << specialFoodTimer << "s) ";
    } else {
        setColor(COLOR_DARKGRAY);
        cout << "Khong co             ";
    }

    setColor(COLOR_BROWN);
    goToXY(infoX, infoY + 11); cout << "-----------------------------";
    goToXY(infoX, infoY + 12); cout << "Dieu khien: W/A/S/D hoac Mui ten";
    goToXY(infoX, infoY + 13); cout << "Phim P    : Tam dung tro choi";
    goToXY(infoX, infoY + 14); cout << "Phim X    : Thoat ve menu chinh";
    goToXY(infoX, infoY + 15); cout << "-----------------------------";
    setColor(COLOR_WHITE);
}

// ==========================================
// HÀM SINH TOẠ ĐỘ VÀ QUẢN LÝ MỒI
// ==========================================

// Sinh mồi thường không bị trùng vào bất kỳ đốt thân rắn nào
void spawnNormalFood(int snakeX[], int snakeY[], int snakeLen, int &foodX, int &foodY) {
    bool onSnake;
    do {
        onSnake = false;
        foodX = rand() % BOARD_WIDTH;
        foodY = rand() % BOARD_HEIGHT;
        for (int i = 0; i < snakeLen; ++i) {
            if (snakeX[i] == foodX && snakeY[i] == foodY) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);

    goToXY(OFFSET_X + 1 + foodX, OFFSET_Y + 1 + foodY);
    setColor(COLOR_LIGHTRED);
    cout << "O";
    setColor(COLOR_WHITE);
}

// Sinh mồi đặc biệt (cộng nhiều điểm hơn, có thời hạn)
void spawnSpecialFood(int snakeX[], int snakeY[], int snakeLen, int foodX, int foodY, int &specX, int &specY) {
    bool invalid;
    do {
        invalid = false;
        specX = rand() % BOARD_WIDTH;
        specY = rand() % BOARD_HEIGHT;
        if (specX == foodX && specY == foodY) invalid = true;
        for (int i = 0; i < snakeLen; ++i) {
            if (snakeX[i] == specX && snakeY[i] == specY) {
                invalid = true;
                break;
            }
        }
    } while (invalid);

    goToXY(OFFSET_X + 1 + specX, OFFSET_Y + 1 + specY);
    setColor(COLOR_YELLOW);
    cout << "$";
    setColor(COLOR_WHITE);
}

// Xoá mồi đặc biệt khi hết thời gian tồn tại
void clearSpecialFood(int specX, int specY) {
    goToXY(OFFSET_X + 1 + specX, OFFSET_Y + 1 + specY);
    cout << " ";
}

// ==========================================
// GAME LOOP VÀ LOGIC CHÍNH
// ==========================================

// Xử lý một màn chơi cụ thể
void playGameSession(bool passWallMode) {
    system("cls");
    setCursorVisible(false);

    // Mảng toạ độ thân rắn (đáp ứng tiêu chí lưu bằng x[], y[])
    int snakeX[MAX_SNAKE_LEN];
    int snakeY[MAX_SNAKE_LEN];
    int snakeLen = 3;

    // Vị trí khởi tạo ban đầu giữa bàn cờ
    snakeX[0] = BOARD_WIDTH / 2;
    snakeY[0] = BOARD_HEIGHT / 2;
    snakeX[1] = snakeX[0] - 1;
    snakeY[1] = snakeY[0];
    snakeX[2] = snakeX[0] - 2;
    snakeY[2] = snakeY[0];

    Direction dir = DIR_RIGHT;
    int score = 0;
    int speedMs = 120; // Độ trễ ban đầu (ms)

    // Khởi tạo trạng thái mồi
    int foodX = 0, foodY = 0;
    int specX = -1, specY = -1;
    bool hasSpecialFood = false;
    int specialFoodTimer = 0;
    clock_t lastSpecCheck = clock();

    drawGameBoard();
    spawnNormalFood(snakeX, snakeY, snakeLen, foodX, foodY);

    // Vẽ thân rắn ban đầu
    for (int i = 0; i < snakeLen; ++i) {
        goToXY(OFFSET_X + 1 + snakeX[i], OFFSET_Y + 1 + snakeY[i]);
        if (i == 0) {
            setColor(COLOR_LIGHTGREEN);
            cout << "@";
        } else {
            setColor(COLOR_GREEN);
            cout << "#";
        }
    }
    setColor(COLOR_WHITE);

    drawGameInfo(score, speedMs, passWallMode, hasSpecialFood, specialFoodTimer);

    bool isGameOver = false;

    // Vòng lặp thời gian thực
    while (!isGameOver) {
        // Đọc phím không chặn (Non-blocking I/O)
        if (_kbhit()) {
            int key = _getch();
            if (key == 224) { // Nhóm phím mũi tên
                key = _getch();
                if (key == 72 && dir != DIR_DOWN) dir = DIR_UP;        // Mũi tên lên
                else if (key == 80 && dir != DIR_UP) dir = DIR_DOWN;   // Mũi tên xuống
                else if (key == 75 && dir != DIR_RIGHT) dir = DIR_LEFT;// Mũi tên trái
                else if (key == 77 && dir != DIR_LEFT) dir = DIR_RIGHT;// Mũi tên phải
            } else {
                char ch = (char)tolower(key);
                if (ch == 'w' && dir != DIR_DOWN) dir = DIR_UP;
                else if (ch == 's' && dir != DIR_UP) dir = DIR_DOWN;
                else if (ch == 'a' && dir != DIR_RIGHT) dir = DIR_LEFT;
                else if (ch == 'd' && dir != DIR_LEFT) dir = DIR_RIGHT;
                else if (ch == 'p') {
                    // Tạm dừng trò chơi
                    goToXY(OFFSET_X + BOARD_WIDTH / 2 - 8, OFFSET_Y + BOARD_HEIGHT / 2);
                    setColor(COLOR_YELLOW, COLOR_BLUE);
                    cout << "  TAM DUNG (Nhan P de tiep tuc)  ";
                    setColor(COLOR_WHITE, COLOR_BLACK);
                    while (true) {
                        if (_kbhit()) {
                            int resumeKey = _getch();
                            if (tolower(resumeKey) == 'p') {
                                goToXY(OFFSET_X + BOARD_WIDTH / 2 - 8, OFFSET_Y + BOARD_HEIGHT / 2);
                                for (int i = 0; i < 33; ++i) cout << " ";
                                break;
                            }
                        }
                        Sleep(50);
                    }
                } else if (ch == 'x') {
                    // Phím thoát khẩn cấp về menu
                    isGameOver = true;
                    break;
                }
            }
        }

        // Đồng hồ đếm ngược và sinh mồi đặc biệt
        clock_t now = clock();
        if (double(now - lastSpecCheck) / CLOCKS_PER_SEC >= 1.0) {
            lastSpecCheck = now;
            if (hasSpecialFood) {
                specialFoodTimer--;
                if (specialFoodTimer <= 0) {
                    hasSpecialFood = false;
                    clearSpecialFood(specX, specY);
                }
                drawGameInfo(score, speedMs, passWallMode, hasSpecialFood, specialFoodTimer);
            } else {
                // Tỉ lệ 10% mỗi giây sẽ sinh mồi đặc biệt
                if (rand() % 10 == 0) {
                    hasSpecialFood = true;
                    specialFoodTimer = 8; // Tồn tại 8 giây
                    spawnSpecialFood(snakeX, snakeY, snakeLen, foodX, foodY, specX, specY);
                    drawGameInfo(score, speedMs, passWallMode, hasSpecialFood, specialFoodTimer);
                }
            }
        }

        // Tính toạ độ mới cho đầu rắn
        int nextX = snakeX[0];
        int nextY = snakeY[0];
        switch (dir) {
            case DIR_UP:    nextY--; break;
            case DIR_DOWN:  nextY++; break;
            case DIR_LEFT:  nextX--; break;
            case DIR_RIGHT: nextX++; break;
        }

        // Kiểm tra va chạm biên tường
        if (passWallMode) {
            // Chế độ xuyên tường: vòng sang mép đối diện
            if (nextX < 0) nextX = BOARD_WIDTH - 1;
            else if (nextX >= BOARD_WIDTH) nextX = 0;
            if (nextY < 0) nextY = BOARD_HEIGHT - 1;
            else if (nextY >= BOARD_HEIGHT) nextY = 0;
        } else {
            // Chế độ cổ điển: đâm tường là thua
            if (nextX < 0 || nextX >= BOARD_WIDTH || nextY < 0 || nextY >= BOARD_HEIGHT) {
                isGameOver = true;
                break;
            }
        }

        // Kiểm tra cắn vào thân rắn
        for (int i = 0; i < snakeLen - 1; ++i) {
            if (nextX == snakeX[i] && nextY == snakeY[i]) {
                isGameOver = true;
                break;
            }
        }
        if (isGameOver) break;

        // Kiểm tra ăn mồi
        bool eatNormal = (nextX == foodX && nextY == foodY);
        bool eatSpecial = (hasSpecialFood && nextX == specX && nextY == specY);

        if (eatNormal || eatSpecial) {
            if (eatNormal) {
                score += 10;
                if (snakeLen < MAX_SNAKE_LEN) snakeLen++;
                spawnNormalFood(snakeX, snakeY, snakeLen, foodX, foodY);
            }
            if (eatSpecial) {
                score += 30;
                hasSpecialFood = false;
                specialFoodTimer = 0;
            }

            // Tăng tốc độ game: giảm thời gian Sleep
            if (speedMs > 35) {
                speedMs -= 3;
            }
            drawGameInfo(score, speedMs, passWallMode, hasSpecialFood, specialFoodTimer);
        } else {
            // Xoá đốt đuôi cũ (kỹ thuật cập nhật điểm ảnh cục bộ, chống nhấp nháy 100%)
            goToXY(OFFSET_X + 1 + snakeX[snakeLen - 1], OFFSET_Y + 1 + snakeY[snakeLen - 1]);
            cout << " ";
        }

        // Dịch chuyển các khúc thân rắn
        for (int i = snakeLen - 1; i > 0; --i) {
            snakeX[i] = snakeX[i - 1];
            snakeY[i] = snakeY[i - 1];
        }
        snakeX[0] = nextX;
        snakeY[0] = nextY;

        // Vẽ lại khúc cổ rắn
        if (snakeLen > 1) {
            goToXY(OFFSET_X + 1 + snakeX[1], OFFSET_Y + 1 + snakeY[1]);
            setColor(COLOR_GREEN);
            cout << "#";
        }

        // Vẽ đầu rắn mới
        goToXY(OFFSET_X + 1 + snakeX[0], OFFSET_Y + 1 + snakeY[0]);
        setColor(COLOR_LIGHTGREEN);
        cout << "@";
        setColor(COLOR_WHITE);

        Sleep(speedMs);
    }

    // Kết thúc màn chơi
    setCursorVisible(true);
    goToXY(OFFSET_X + BOARD_WIDTH / 2 - 7, OFFSET_Y + BOARD_HEIGHT / 2 - 1);
    setColor(COLOR_LIGHTRED, COLOR_BLACK);
    cout << "=== GAME OVER ===";

    goToXY(OFFSET_X + BOARD_WIDTH / 2 - 10, OFFSET_Y + BOARD_HEIGHT / 2 + 1);
    setColor(COLOR_YELLOW);
    cout << "Diem dat duoc: " << score;

    setColor(COLOR_WHITE);
    goToXY(OFFSET_X + 2, OFFSET_Y + BOARD_HEIGHT + 3);
    cout << "Nhap ten cua ban (viet lien khong dau): ";
    string name;
    cin >> name;
    if (!name.empty()) {
        saveHighScore(name, score);
    }

    goToXY(OFFSET_X + 2, OFFSET_Y + BOARD_HEIGHT + 5);
    cout << "Nhan phim bat ky de quay lai menu chinh...";
    _getch();
}

// ==========================================
// CÁC MÀN HÌNH CHỨC NĂNG PHỤ
// ==========================================

// Hiển thị bảng xếp hạng điểm cao từ file
void showHighScoresScreen() {
    system("cls");
    setColor(COLOR_YELLOW);
    cout << "=====================================================\n";
    cout << "             BANG XEP HANG DIEM CAO                  \n";
    cout << "=====================================================\n";
    setColor(COLOR_WHITE);

    vector<HighScore> list = loadHighScores();
    if (list.empty()) {
        cout << "  (Chua co du lieu ky luc nao duoc ghi nhan)\n";
    } else {
        cout << "  Top   Nguoi choi                 Diem so\n";
        cout << "  -----------------------------------------\n";
        for (size_t i = 0; i < list.size(); ++i) {
            setColor(i == 0 ? COLOR_LIGHTRED : COLOR_LIGHTCYAN);
            printf("  #%-4d %-25s %d\n", (int)(i + 1), list[i].playerName.c_str(), list[i].score);
        }
    }

    setColor(COLOR_BROWN);
    cout << "\n=====================================================\n";
    cout << "Nhan phim bat ky de quay lai menu chinh...";
    setColor(COLOR_WHITE);
    _getch();
}

// Hiển thị hướng dẫn luật chơi
void showInstructionsScreen() {
    system("cls");
    setColor(COLOR_LIGHTCYAN);
    cout << "=====================================================\n";
    cout << "              HUONG DAN LUAT CHOI RAN SAN MOI        \n";
    cout << "=====================================================\n";
    setColor(COLOR_WHITE);
    cout << " 1. Dieu khien con ran:\n";
    cout << "    - Phim W hoac Mui ten Len   : Di chuyen LEN\n";
    cout << "    - Phim S hoac Mui ten Xuong : Di chuyen XUONG\n";
    cout << "    - Phim A hoac Mui ten Trai  : Di chuyen SANG TRAI\n";
    cout << "    - Phim D hoac Mui ten Phai  : Di chuyen SANG PHAI\n";
    cout << "    * Quy tac: Ran khong the quay nguoc 180 do.\n\n";

    cout << " 2. He thong thuc an:\n";
    cout << "    - Moi thuong ('O')  : +10 diem, ran dai them 1 dot.\n";
    cout << "    - Moi dac biet ('$'): +30 diem, bien mat sau vai giay!\n\n";

    cout << " 3. Che do va Quy tac thua:\n";
    cout << "    - Dam vao than ran: Thua cuoc.\n";
    cout << "    - Che do Co dien: Dam vao tuong bien la thua.\n";
    cout << "    - Che do Xuyen tuong: Ran di xuyen qua mep doi dien.\n";
    cout << "    - Phim P: Tam dung game khi can.\n";

    setColor(COLOR_BROWN);
    cout << "=====================================================\n";
    cout << "Nhan phim bat ky de quay lai menu chinh...";
    setColor(COLOR_WHITE);
    _getch();
}

// Hàm kiểm tra hợp lệ dữ liệu nhập (chống crash khi nhập chuỗi ký tự)
int getValidMenuChoice(int minVal, int maxVal) {
    int choice;
    while (true) {
        cout << ">> Lua chon cua ban [" << minVal << " - " << maxVal << "]: ";
        if (cin >> choice) {
            if (choice >= minVal && choice <= maxVal) {
                return choice;
            }
            setColor(COLOR_LIGHTRED);
            cout << "Loi: Gia tri nam ngoai khoang cho phep. Vui long chon lai!\n";
            setColor(COLOR_WHITE);
        } else {
            setColor(COLOR_LIGHTRED);
            cout << "Loi: Dinh dang khong hop le. Vui long chi nhap so nguyen!\n";
            setColor(COLOR_WHITE);
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        }
    }
}

// ==========================================
// HÀM MAIN: ĐIỀU PHỐI MENU CHÍNH
// ==========================================
int main() {
    // Khởi tạo seed ngẫu nhiên
    srand((unsigned int)time(NULL));

    // Đặt tên tiêu đề cửa sổ console
    SetConsoleTitleA("Game Ran San Moi C++ - Console Edition");

    bool running = true;

    while (running) {
        system("cls");
        setCursorVisible(true);
        setColor(COLOR_LIGHTGREEN);
        cout << "=====================================================\n";
        cout << "          TRO CHOI RAN SAN MOI (SNAKE GAME)          \n";
        cout << "=====================================================\n";
        setColor(COLOR_WHITE);
        cout << "  1. Choi game - Che do Co dien (Dam tuong la chet)\n";
        cout << "  2. Choi game - Che do Nang cao (Xuyen tuong)\n";
        cout << "  3. Bang xep hang ky luc\n";
        cout << "  4. Huong dan choi\n";
        cout << "  5. Thoat tro choi\n";
        setColor(COLOR_LIGHTGREEN);
        cout << "-----------------------------------------------------\n";
        setColor(COLOR_WHITE);

        int choice = getValidMenuChoice(1, 5);

        switch (choice) {
            case 1:
                playGameSession(false); // Chế độ va tường thua
                break;
            case 2:
                playGameSession(true);  // Chế độ xuyên tường
                break;
            case 3:
                showHighScoresScreen();
                break;
            case 4:
                showInstructionsScreen();
                break;
            case 5:
                running = false;
                system("cls");
                setColor(COLOR_LIGHTCYAN);
                cout << "\nCam on ban da trai nghiem tro choi! Tam biet!\n\n";
                setColor(COLOR_WHITE);
                break;
        }
    }

    return 0;
}
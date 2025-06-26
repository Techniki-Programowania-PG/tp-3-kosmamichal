// ======== Techniki programowania - Projekt 4 ======== //

#include <windows.h>
#include <gdiplus.h>
#include <vector>

using namespace Gdiplus;
using namespace std;

#pragma comment(lib, "Gdiplus.lib")

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define BLOCK_W 60
#define BLOCK_H 60
#define GROUND_Y 500
#define MAX_STACK 3

enum ShapeType { SQUARE, TRIANGLE };

struct Shape {
    ShapeType type;
    RECT rect;
    bool picked;
};

HINSTANCE hInst;
ULONG_PTR gdiToken;
vector<Shape> blocks;
vector<ShapeType> stack;
int pos_x = 50;
int pos_y = 50;
char mode = 'Q'; 

bool IsOverlapping(const RECT& a, const RECT& b) {
    return !(a.right <= b.left || a.left >= b.right || a.bottom <= b.top || a.top >= b.bottom);
}

ATOM RegisterWindowClass(HINSTANCE hInstance);
bool InitWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    hInst = hInstance;
    GdiplusStartupInput gdiplusInput;
    GdiplusStartup(&gdiToken, &gdiplusInput, NULL);

    RegisterWindowClass(hInstance);
    if (!InitWindow(hInstance, nCmdShow)) return FALSE;

    for (int i = 0; i < 8; i++) {
        RECT r = { 50 + i * 100, GROUND_Y, 50 + i * 100 + BLOCK_W, GROUND_Y + BLOCK_H };
        ShapeType ksztalt;
        if (i % 2 == 0) {
            ksztalt = SQUARE;
        }
        else {
            ksztalt = TRIANGLE;
        }
        blocks.push_back({ ksztalt, r, false });
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiToken);
    return (int)msg.wParam;
}

ATOM RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CraneSim";
    return RegisterClassExW(&wc);
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow) {
    HWND hWnd = CreateWindowW(L"CraneSim", L"Symulator dźwigu", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

    if (!hWnd) return false;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    SetTimer(hWnd, 1, 30, NULL);
    return true;
}

void DrawShape(Graphics& g, Shape& s, SolidBrush& red, SolidBrush& blue) {
    if (s.type == SQUARE) {
        g.FillRectangle(&red, Rect(s.rect.left, s.rect.top, BLOCK_W, BLOCK_H));
    }
    else {
        Point points[3] = {
            { s.rect.left + BLOCK_W / 2, s.rect.top },
            { s.rect.left, s.rect.top + BLOCK_H },
            { s.rect.left + BLOCK_W, s.rect.top + BLOCK_H }
        };
        g.FillPolygon(&blue, points, 3);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_TIMER:
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        if (wp == 'A') pos_x = max(0, pos_x - 10);
        if (wp == 'D') pos_x = min(WINDOW_WIDTH - BLOCK_W, pos_x + 10);
        if (wp == 'W') pos_y = max(0, pos_y - 10);

        if (wp == 'S') {
            int limitY = GROUND_Y;
            for (auto& s : blocks) {
                if (!s.picked && s.rect.left == pos_x) {
                    limitY = min(limitY, s.rect.top);
                }
            }
            pos_y = min(limitY - (int)stack.size() * BLOCK_H + 1, pos_y + 10);
        }

        if (wp == 'M') {
            if (mode == 'Q') {
                mode = 'T';
            }
            else {
                mode = 'Q';
            }
        }


        if (wp == 'P') {
            if (stack.size() >= MAX_STACK) break;
            RECT hook = { pos_x, pos_y, pos_x + BLOCK_W, pos_y + BLOCK_H };

            for (int i = (int)blocks.size() - 1; i >= 0; --i) {
                auto& s = blocks[i];
                if (s.picked) continue;

                RECT fullStack = { pos_x, pos_y, pos_x + BLOCK_W, pos_y + (int)stack.size() * BLOCK_H + BLOCK_H };
                if (IsOverlapping(fullStack, s.rect) && ((mode == 'Q' && s.type == SQUARE) || (mode == 'T' && s.type == TRIANGLE))) {
                    s.picked = true;
                    stack.push_back(s.type);
                    break;
                }
            }
        }

        if (wp == 'X') {
            if (stack.empty()) break;

            RECT drop = { pos_x, 0, pos_x + BLOCK_W, GROUND_Y + BLOCK_H };
            int top = GROUND_Y + BLOCK_H;
            int count = 0;
            bool wrong_place = false;

            for (auto& s : blocks) {
                if (s.picked) continue;
                if (IsOverlapping(drop, s.rect)) {
                    if (s.type != stack[0]) {
                        count = -999;
                        break;
                    }
                    count++;
                    top = min(top, s.rect.top);
                    if (s.rect.left != pos_x) wrong_place = true;
                }
            }

            if (count < 0 || count + stack.size() > MAX_STACK || wrong_place) break;

            for (size_t i = 0; i < stack.size(); ++i) {
                int y = top - (int)(i + 1) * BLOCK_H;
                RECT r = { pos_x, y, pos_x + BLOCK_W, y + BLOCK_H };
                blocks.push_back({ stack[i], r, false });
            }
            stack.clear();
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Bitmap buffer(WINDOW_WIDTH, WINDOW_HEIGHT, PixelFormat32bppARGB);
        Graphics g(&buffer);

        SolidBrush white(Color(255, 255, 255));
        SolidBrush red(Color(255, 0, 0));
        SolidBrush blue(Color(0, 0, 255));
        SolidBrush gray(Color(200, 200, 200));
        SolidBrush panel(Color(240, 240, 240));
        SolidBrush black(Color(0, 0, 0));
        Pen pen(Color(0, 0, 0), 3);

        g.FillRectangle(&white, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        g.FillRectangle(&gray, 0, GROUND_Y + BLOCK_H, WINDOW_WIDTH, WINDOW_HEIGHT);

        for (auto& s : blocks) {
            if (!s.picked) {
                DrawShape(g, s, red, blue);
            } 
        }

        g.DrawLine(&pen, pos_x + BLOCK_W / 2, 0, pos_x + BLOCK_W / 2, pos_y);
        g.DrawRectangle(&pen, Rect(pos_x, pos_y, BLOCK_W, BLOCK_H));

        for (size_t i = 0; i < stack.size(); ++i) {
            int y = pos_y + BLOCK_H + (int)i * BLOCK_H;
            Shape s = { stack[i], { pos_x, y, pos_x + BLOCK_W, y + BLOCK_H }, false };
            DrawShape(g, s, red, blue);
        }

        int panel_x = WINDOW_WIDTH - 325;
        g.FillRectangle(&panel, panel_x, 5, 300, 180);
        g.DrawRectangle(&pen, Rect(panel_x, 5, 300, 180));

        FontFamily fontFamily(L"Verdana");
        Font font(&fontFamily, 16, FontStyleBold, UnitPixel);

        const WCHAR* text[] = { L"A: Lewo", L"D: Prawo", L"W: Góra", L"S: Dół", L"M: Zmień tryb", L"P: Podnieś", L"X: Opuść" };
        for (int i = 0; i < 6; i++) {
            g.DrawString(text[i], -1, &font, PointF(panel_x + 10, 15 + i * 20), &black);
        }

        WCHAR modeStr[64];

        const WCHAR* wybrany_tryb;
        if (mode == 'Q') {
            wybrany_tryb = L"Kwadrat";
        }
        else {
            wybrany_tryb = L"Trójkąt";
        }
        wsprintf(modeStr, L"Wybrany tryb: %s", wybrany_tryb);

        g.DrawString(modeStr, -1, &font, PointF(panel_x + 10, 140), &black);

        Graphics screen(hdc);
        screen.DrawImage(&buffer, 0, 0);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, msg, wp, lp);
    }
}
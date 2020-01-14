#include <Windows.h>

#include "game.h"

#pragma warning(disable : 28251) // SAL annotation


static bool running = true;
static f64 performance_frequency;




u64 TimeStamp()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}
f64 TimeElapsed(u64 start, u64 end)
{
	return ((end - start) / performance_frequency);
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
	case WM_CREATE: // 0
	{
	} break;

	case WM_PAINT: // 0
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	} break;

	case WM_CLOSE: // 0
	{
		DestroyWindow(hwnd);
		PostQuitMessage(0);
	} break;

	case WM_SIZE: // 0
	{} break;

	default:
	{
		result = DefWindowProc(hwnd, msg, wparam, lparam);
	} break;
	}
	return result;
}


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR lpcmdline, int)
{
	lpcmdline;

	int window_width = 256;
	int window_height = 240;

	double update_interval = 1.0 / 60.0;


	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = TEXT("wndclass");
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (RegisterClassEx(&wc))
	{
		DWORD style = (WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_VISIBLE) & ~WS_SIZEBOX; // No resizing
		RECT window_rect = { 0, 0, window_width, window_height };
		AdjustWindowRectEx(&window_rect, style, FALSE, 0);
		int window_x = (GetSystemMetrics(SM_CXSCREEN) - (window_rect.right - window_rect.left)) / 2;
		int window_y = (GetSystemMetrics(SM_CYSCREEN) - (window_rect.bottom - window_rect.top)) / 2;
		HWND hwnd = CreateWindowEx(0, TEXT("wndclass"), TEXT("Demo"), style, window_x, window_y, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, NULL, NULL, hinstance, NULL);
		if (hwnd)
		{
			HDC hdc = GetDC(hwnd);


			// @TODO: Framework initialization
			// renderer_initialize(window_width, window_height);

			LARGE_INTEGER pf;
			QueryPerformanceFrequency(&pf);
			performance_frequency = (f64)pf.QuadPart;

			u64 update_timer_start = TimeStamp();
			int update_counter = 0;
			double update_accumulator = 0.0f;
			int render_counter = 0;
			u64 per_second_timer_start = TimeStamp();

			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);

			while (running)
			{
				MSG msg;
				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (msg.message == WM_QUIT)
					{
						running = false;
					}
				}


				// Update
				u64 update_timer_now = TimeStamp();
				update_accumulator += TimeElapsed(update_timer_start, update_timer_now);
				while (update_accumulator >= update_interval)
				{
					++update_counter;
					update_accumulator -= update_interval;
					GameUpdate(update_interval);
				}
				update_timer_start = update_timer_now;


				// Render
				int source_width, source_height;
				UINT32 *pixels = GameRender(window_width, window_height, &source_width, &source_height);
				BITMAPINFO bitmapinfo = {};
				bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bitmapinfo.bmiHeader.biWidth = source_width;
				bitmapinfo.bmiHeader.biHeight = -source_height;
				bitmapinfo.bmiHeader.biPlanes = 1;
				bitmapinfo.bmiHeader.biBitCount = 32;
				bitmapinfo.bmiHeader.biCompression = BI_RGB;
				StretchDIBits(hdc, 0, 0, window_width, window_height, 0, 0, source_width, source_height, pixels, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
				++render_counter;


				// Per second information
				u64 per_second_timer_now = TimeStamp();
				if (TimeElapsed(per_second_timer_start, per_second_timer_now) >= 1.0)
				{
					TCHAR buf[0x100];
					wsprintf(buf, TEXT("UPS: %d, FPS: %d"), update_counter, render_counter);
					SetWindowText(hwnd, buf);

					update_counter = 0;
					render_counter = 0;
					per_second_timer_start = per_second_timer_now;
				}
			}

			// @TODO: Shutdown
			// application_shutdown();
		}
	}
	return 0;
}

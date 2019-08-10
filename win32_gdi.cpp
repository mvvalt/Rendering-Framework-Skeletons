/*
@TODO: Fix AdjustWindowRectEx (see win32_opengl.cpp)
*/
#include <Windows.h>

bool g_running = true;
LARGE_INTEGER g_performance_frequency = {};


float elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end)
{
	return (float)((end.QuadPart - start.QuadPart) / (double)g_performance_frequency.QuadPart);
}


LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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

	int window_width = 1280;
	int window_height = 720;

	const int source_width = 256;
	const int source_height = 240;

	float update_interval = 1.0f / 60.0f;


	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = "wndclass";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (RegisterClassEx(&wc))
	{
		DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		RECT window_rect = { 0, 0, window_width, window_height };
		AdjustWindowRectEx(&window_rect, style, FALSE, 0);
		HWND hwnd = CreateWindowEx(0, "wndclass", "Demo", style, CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, NULL, NULL, hinstance, NULL);
		if (hwnd)
		{
			BITMAPINFO bitmapinfo = {};
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapinfo.bmiHeader.biWidth = source_width;
			bitmapinfo.bmiHeader.biHeight = -source_height;
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biBitCount = 32;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;

			HDC hdc = GetDC(hwnd);
			

			// @TODO: Framework initialization
			// renderer_initialize(window_width, window_height);


			LARGE_INTEGER update_timer_start;
			int update_counter = 0;
			float update_accumulator = 0.0f;
			int render_counter = 0;
			LARGE_INTEGER per_second_timer_start;

			QueryPerformanceFrequency(&g_performance_frequency);
			QueryPerformanceCounter(&update_timer_start);
			QueryPerformanceCounter(&per_second_timer_start);

			// @TODO: Initialization
			// application_initialize(window_width, window_height);


			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);

			while (g_running)
			{
				MSG msg;
				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (msg.message == WM_QUIT)
					{
						g_running = false;
					}
				}


				// Update
				LARGE_INTEGER update_timer_now;
				QueryPerformanceCounter(&update_timer_now);
				update_accumulator += elapsed_time(update_timer_start, update_timer_now);
				while (update_accumulator >= update_interval)
				{
					++update_counter;
					update_accumulator -= update_interval;

					// @TODO: Update
					// application_update(update_interval);
				}
				update_timer_start = update_timer_now;


				// Render

				// @TODO: Render
				// application_render();

				// @TODO: Get the pixels that we want to display on the screen
				// unsigned int *pixels = get_pixels();
				UINT32 pixels[source_width * source_height];
				memset(pixels, 0x55, source_width * source_height * sizeof(UINT32));
				StretchDIBits(hdc, 0, 0, window_width, window_height, 0, 0, source_width, source_height, pixels, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
								
				++render_counter;


				// Per second information
				LARGE_INTEGER per_second_timer_now;
				QueryPerformanceCounter(&per_second_timer_now);
				if (elapsed_time(per_second_timer_start, per_second_timer_now) >= 1.0f)
				{
					char buf[0x100];
					wsprintf(buf, "UPS: %d, FPS: %d", update_counter, render_counter);
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

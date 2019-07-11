#include <Windows.h>
#include <gl/GL.h>
#include <gl/wglext.h>


bool g_running = true;

HDC g_hdc = {};
HGLRC g_hglrc = {};
LARGE_INTEGER g_performance_frequency = {};


float elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end)
{
	return (float)((end.QuadPart - start.QuadPart) / (double)g_performance_frequency.QuadPart);
}


// A more robust program would load individual extensions and keep them on hand,
// but vsync is not a critical operation and we can just get a handle to the
// function each time we want to use it.
// https://stackoverflow.com/questions/34063022/wglext-extension-not-installed-in-opengl-context
void set_vsync(int interval)
{
	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(interval);
	}
}


LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
		case WM_CREATE:
		{
			g_hdc = GetDC(hwnd);

			PIXELFORMATDESCRIPTOR pfd;
			int pixelformat;

			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.dwLayerMask = PFD_MAIN_PLANE;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cAlphaBits = 8;

			pixelformat = ChoosePixelFormat(g_hdc, &pfd);
			if (pixelformat)
			{
				if (SetPixelFormat(g_hdc, pixelformat, &pfd))
				{
					g_hglrc = wglCreateContext(g_hdc);
					wglMakeCurrent(g_hdc, g_hglrc);
				}
				else
				{
					PostQuitMessage(-2);
				}
			}
			else
			{
				PostQuitMessage(-1);
			}
		} break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		} break;
		
		case WM_CLOSE:
		{
			if (g_hglrc)
			{
				wglDeleteContext(g_hglrc);
			}
			if (g_hdc)
			{
				ReleaseDC(hwnd, g_hdc);
			}
			DestroyWindow(hwnd);
			g_running = false;
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

	float update_interval = 1.0f / 30.0f;


	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
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
			set_vsync(1);

			while (g_running)
			{
				MSG msg;
				while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
				{	
					TranslateMessage(&msg);
					DispatchMessage(&msg);
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
				glViewport(0, 0, 640, 360);
				glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				// @TODO: Render
				// application_render();
				
				// @TODO: Get the pixels that we want to display on the screen
				// unsigned int *buffer = renderer_get_buffer();
				// glDrawPixels(window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
				
				SwapBuffers(g_hdc);
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

// Big thanks to https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
#include <Windows.h>

#include <glad/glad.h>
#include <gl/wglext.h>

#include <stdio.h> // freopen_s


PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;


LARGE_INTEGER g_performance_frequency = {};


bool running = true;


// A critical error caused by one of the operating system calls failing
void system_error(const char *msg)
{
	MessageBoxEx(NULL, msg, TEXT("System Error"), MB_OK | MB_ICONERROR, 0);
	// @TODO: log to file
}


// An opengl error
void APIENTRY opengl_error(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param)
{
	source;
	type;
	id;
	severity;
	length;
	//message;
	user_param;

	MessageBoxEx(NULL, (char *)message, TEXT("Opengl Error"), MB_OK | MB_ICONERROR, 0);
	// @TODO: log to file
}


// @NOTE: This is not a critical operation, so we just get the
// function pointer on demand.
void toggle_vsync(bool enabled)
{
	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
	{
		if (enabled)
		{
			wglSwapIntervalEXT(1);
		}
		else
		{
			wglSwapIntervalEXT(0);
		}
	}
}


float elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end)
{
	return (float)((end.QuadPart - start.QuadPart) / (double)g_performance_frequency.QuadPart);
}


LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
			PostQuitMessage(0);
		} break;

		case WM_SIZE: // 0
		{
			//renderer::window_resize(LOWORD(lparam), HIWORD(lparam));
		} break;

		default:
		{
			result = DefWindowProc(hwnd, msg, wparam, lparam);
		} break;
	}

	return result;
}


struct WindowData
{
	HWND hwnd;
	HDC hdc;
	HGLRC hglrc;
};


struct ConfigOptions
{
	// Window
	bool fullscreen; // Do we run the game fullscreen at the desktop resolution?
	int fullscreen_width;
	int fullscreen_height;
	int windowed_width;
	int windowed_height;
	int multisample_samples;
	bool vsync;

	// Misc
	float update_interval;
};


bool create_window(HINSTANCE hinstance, ConfigOptions &config, WindowData &wnd_data)
{
	bool result = false;

	int opengl_version_major = 4;
	int opengl_version_minor = 5;

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = "window class";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (RegisterClassEx(&wc))
	{
		DWORD style = 0;
		RECT window_rect;
		int primary_screen_width = GetSystemMetrics(SM_CXSCREEN);
		int primary_screen_height = GetSystemMetrics(SM_CYSCREEN);
		int wx = 0;
		int wy = 0;
		if (config.fullscreen)
		{
			config.fullscreen_width = primary_screen_width;
			config.fullscreen_height = primary_screen_height;
			style = WS_POPUP;
			window_rect = { 0, 0, config.fullscreen_width, config.fullscreen_height };
		}
		else
		{
			style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU; // No min/max buttons
			//style |= WS_THICKFRAME; // Resizing the window manually is allowed
			window_rect = { 0, 0, config.windowed_width, config.windowed_height };
			AdjustWindowRectEx(&window_rect, style, FALSE, 0);
			wx = (primary_screen_width - (window_rect.right - window_rect.left)) / 2;
			wy = (primary_screen_height - (window_rect.bottom - window_rect.top)) / 2;
		}

		wnd_data.hwnd = CreateWindowEx(0, wc.lpszClassName, "Demo", style, wx, wy, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, NULL, NULL, hinstance, NULL);
		if (wnd_data.hwnd)
		{
			wnd_data.hdc = GetDC(wnd_data.hwnd);

			const int pixel_attrib[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB, 24,
				WGL_ALPHA_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				WGL_SAMPLE_BUFFERS_ARB, 1,			// multisampling enabled
				WGL_SAMPLES_ARB, config.multisample_samples,	// multisampling samples (e.g. 4x, 8x)
				0
			};
			int pf_id;
			UINT num_pixelformats;
			bool status = wglChoosePixelFormatARB(wnd_data.hdc, pixel_attrib, NULL, 1, &pf_id, &num_pixelformats);
			if (status && num_pixelformats)
			{
				PIXELFORMATDESCRIPTOR pfd;
				DescribePixelFormat(wnd_data.hdc, pf_id, sizeof(pfd), &pfd);
				SetPixelFormat(wnd_data.hdc, pf_id, &pfd);

				int ctx_attrib[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, opengl_version_major,
					WGL_CONTEXT_MINOR_VERSION_ARB, opengl_version_minor,
					WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
					0
				};

				wnd_data.hglrc = wglCreateContextAttribsARB(wnd_data.hdc, 0, ctx_attrib);
				if (wnd_data.hglrc)
				{
					// Success
					result = true;
				}
				else
				{
					system_error("wglCreateContextAttribsARB().");
				}
			}
			else
			{
				system_error("wglChoosePixelFormatARB().");
			}
		}
		else
		{
			system_error("CreateWindowEx().");
		}
	}
	else
	{
		system_error("RegisterClassEx().");
	}

	return result;
}
bool create_temporary_window(HINSTANCE hinstance, WindowData &wnd_data)
{
	bool result = false;

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc; // We don't need a real window procedure
	wc.hInstance = hinstance;
	wc.lpszClassName = "temporary window class";
	if (RegisterClassEx(&wc))
	{
		wnd_data.hwnd = CreateWindowEx(0, wc.lpszClassName, "", WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 1, 1, NULL, NULL, hinstance, NULL);
		if (wnd_data.hwnd)
		{
			wnd_data.hdc = GetDC(wnd_data.hwnd); // Would this ever fail?

			PIXELFORMATDESCRIPTOR temp_pfd = {};
			temp_pfd.nSize = sizeof(temp_pfd);
			temp_pfd.nVersion = 1;
			temp_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			temp_pfd.iPixelType = PFD_TYPE_RGBA;
			temp_pfd.cColorBits = 24;
			temp_pfd.cAlphaBits = 8;
			temp_pfd.cDepthBits = 24;
			temp_pfd.cStencilBits = 8;

			int temp_pfd_id = ChoosePixelFormat(wnd_data.hdc, &temp_pfd);
			if (temp_pfd_id)
			{
				if (SetPixelFormat(wnd_data.hdc, temp_pfd_id, &temp_pfd))
				{
					wnd_data.hglrc = wglCreateContext(wnd_data.hdc);
					if (wnd_data.hglrc)
					{
						if (wglMakeCurrent(wnd_data.hdc, wnd_data.hglrc))
						{
							// We need to get pointers to these functions manually
							wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
							wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
							if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
							{
								// Success
								result = true;
							}
							else
							{
								system_error("wglGetProcAddress().");
							}
						}
						else
						{
							system_error("wglMakeCurrent().");
						}
					}
					else
					{
						system_error("wglCreateContext().");
					}
				}
				else
				{
					system_error("SetPixelFormat().");
				}
			}
			else
			{
				system_error("ChoosePixelFormat().");
			}
		}
		else
		{
			system_error("CreateWindowEx().");
		}
	}
	else
	{
		system_error("RegisterClassEx().");
	}

	return result;
}
bool initialize_window(HINSTANCE hinstance, ConfigOptions &config, WindowData &wnd_data)
{
	bool result = false;

	WindowData temp_wnd_data;

	// Create temporary window and temporary opengl context
	if (create_temporary_window(hinstance, temp_wnd_data))
	{
		// Create the real window and opengl context
		if (create_window(hinstance, config, wnd_data))
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(temp_wnd_data.hglrc);
			ReleaseDC(temp_wnd_data.hwnd, temp_wnd_data.hdc);
			DestroyWindow(temp_wnd_data.hwnd);
			if (wglMakeCurrent(wnd_data.hdc, wnd_data.hglrc))
			{
				// Load opengl functions using glad
				gladLoadGL();

				// Set vsync
				toggle_vsync(config.vsync);

				// Opengl debug callback
				glDebugMessageCallback(opengl_error, nullptr);
				glEnable(GL_DEBUG_OUTPUT);

				result = true;
			}
			else
			{
				system_error("wglMakeCurrent().");
			}
		}
		else
		{
			system_error("real window");
		}
	}
	else
	{
		system_error("temporary window");
	}

	return result;
}


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR lpcmdline, int)
{
	int result = -1;


	// @TODO: load config from file
	ConfigOptions config = {};
	config.fullscreen = false;
	config.fullscreen_width = config.fullscreen_height = -1; // These are set to the desktop resolution
	config.windowed_width = 1280;
	config.windowed_height = 720;
	config.multisample_samples = 4;
	config.vsync = true;
	config.update_interval = 1.0f / 30.0f;


	// @TODO: command line
	lpcmdline;


	WindowData window_data = {};
	if (initialize_window(hinstance, config, window_data))
	{

#ifdef GD_LOGTOCONSOLE
		AllocConsole();
		freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
#endif

		if (config.fullscreen)
		{
			//renderer::initialize(config.fullscreen_width, config.fullscreen_height);
		}
		else
		{
			//renderer::initialize(config.windowed_width, config.windowed_height);
		}


		// @TEMP: Show us what version of opengl we're using
		SetWindowText(window_data.hwnd, (LPCSTR)glGetString(GL_VERSION));


		QueryPerformanceFrequency(&g_performance_frequency);

		LARGE_INTEGER update_timer_start;
		int update_counter = 0;
		float update_accumulator = 0.0f;
		int render_counter = 0;
		LARGE_INTEGER per_second_timer_start;

		QueryPerformanceCounter(&update_timer_start);
		QueryPerformanceCounter(&per_second_timer_start);

		ShowWindow(window_data.hwnd, SW_SHOW);
		result = 0;

		while (running)
		{
			MSG msg;
			while (PeekMessage(&msg, window_data.hwnd, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					running = false;
				}
			}


			// Update
			LARGE_INTEGER update_timer_now;
			QueryPerformanceCounter(&update_timer_now);
			update_accumulator += elapsed_time(update_timer_start, update_timer_now);
			while (update_accumulator >= config.update_interval)
			{
				++update_counter;
				update_accumulator -= config.update_interval;

				// @TODO: application_update(config.update_interval);
			}
			update_timer_start = update_timer_now;


			// Render
			//renderer::draw();

			SwapBuffers(window_data.hdc);
			++render_counter;


			// Per second information
			LARGE_INTEGER per_second_timer_now;
			QueryPerformanceCounter(&per_second_timer_now);
			if (elapsed_time(per_second_timer_start, per_second_timer_now) >= 1.0f)
			{
				char buf[0x100];
				wsprintf(buf, "UPS: %d, FPS: %d", update_counter, render_counter);
				SetWindowText(window_data.hwnd, buf);

				update_counter = 0;
				render_counter = 0;
				per_second_timer_start = per_second_timer_now;
			}
		}
	}

	return result;
}

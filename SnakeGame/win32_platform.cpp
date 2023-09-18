#include <windows.h>

static bool running = true;

struct App_Window {
	int height, width;
	void* memory;

	BITMAPINFO bitmapinfo;
};

App_Window app_window;

#include "platform_common.cpp"
#include "utils.cpp"
#include "renderer.cpp"
#include "game.cpp"


LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (uMsg) {
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
		} break;

		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);
			app_window.width = rect.right - rect.left;
			app_window.height = rect.bottom - rect.top;

			int size = app_window.width * app_window.height * sizeof(u32);

			if (app_window.memory) VirtualFree(app_window.memory, 0, MEM_RELEASE);
			app_window.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			app_window.bitmapinfo.bmiHeader.biSize = sizeof(app_window.bitmapinfo.bmiHeader);
			app_window.bitmapinfo.bmiHeader.biWidth = app_window.width;
			app_window.bitmapinfo.bmiHeader.biHeight = app_window.height;
			app_window.bitmapinfo.bmiHeader.biPlanes = 1;
			app_window.bitmapinfo.bmiHeader.biBitCount = 32;
			app_window.bitmapinfo.bmiHeader.biCompression = BI_RGB;
		} break;

		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	// Create Window Class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = L"Game Window Class";
	window_class.lpfnWndProc = window_callback;

	// Register Class
	RegisterClass(&window_class);

	// Create Window
	HWND window = CreateWindow(window_class.lpszClassName, L"Snake Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0); {
		// Fullscreen
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_MAXIMIZE);
		// GET USER MONITOR INFO
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right -
			mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER |
			SWP_FRAMECHANGED);
	}

	HDC hdc = GetDC(window);

	Input input = {};

	// Finding delta_time
	// If going 60FPS the delta_time would be 0.0166666...
	float delta_time = .0f;
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	// Calls how many cycles have occured in one second
	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}

	while (running) {
		// Input
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}

		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			switch (message.message) {
				case WM_KEYUP:
				case WM_KEYDOWN: {
					u32 vk_code = (u32)message.wParam;
					bool is_down = ((message.lParam & (1 << 31)) == 0);

					#define process_button(b, vk) \
						case vk: {\
						input.buttons[b].is_down = is_down;\
						input.buttons[b].changed = true;\
						} break;

					switch (vk_code) {
						process_button(BUTTON_UP, VK_UP);
						process_button(BUTTON_DOWN, VK_DOWN);
						process_button(BUTTON_LEFT, VK_LEFT);
						process_button(BUTTON_RIGHT, VK_RIGHT);
					}
				}break;

				default: {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}

		// Simulate
		simulate_game(&input, delta_time);

		// Render
		// Creates a black screen when the memory is zeroed out
		StretchDIBits(hdc, 0, 0, app_window.width, app_window.height, 0, 0, app_window.width, app_window.height, app_window.memory, &app_window.bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
	
		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		frame_begin_time = frame_end_time;
	}
}
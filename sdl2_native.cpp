#include <SDL.h>


//-----------------------------------------------------------------------------
// @TODO: Place these in a header file included from this source file, with the
// prototypes renamed appropriately
struct Input
{
	// @TODO: mouse, keyboard
};

void Stub_Update(Input& input, float delta_time);
Uint32* Stub_Render(int& output_width, int& output_height);
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// @TODO: Replace this section with your own code, and rename the function
// calls to the Stub_*() functions inside of main().
int stub_framebuffer_width = 800;
int stub_framebuffer_height = 450;

void Stub_Update(Input& input, float delta_time)
{
	input;
	delta_time;
}

// output_width/output_height are the dimensions of the framebuffer that gets resized to fit the application window.
// Returns a pointer to the pixel data (of size output_width * output_height) that will cover the application window.
Uint32* Stub_Render(int& output_width, int& output_height)
{
	static Uint32* buf;
	if (buf == nullptr)
	{
		buf = new Uint32[(size_t)stub_framebuffer_width * (size_t)stub_framebuffer_height];
		for (int y = 0; y < stub_framebuffer_height; ++y)
		{
			for (int x = 0; x < stub_framebuffer_width; ++x)
			{
				buf[y * stub_framebuffer_width + x] = 0xFF333300;
			}
		}
	}
	output_width = stub_framebuffer_width;
	output_height = stub_framebuffer_height;
	return buf;
}
//-----------------------------------------------------------------------------


static bool running = true;
static double performance_frequency;


double ElapsedTime(Uint64 start, Uint64 end)
{
	return (double)(end - start) / performance_frequency;
}


int main(int argc, char* argv[])
{
	argc;
	argv;

	int result = -1;

	int window_width_initial = 800;
	int window_height_initial = 450;
	double update_period = 1.0 / 60.0;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		SDL_Window* sdl_window = SDL_CreateWindow("software renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_initial, window_height_initial, SDL_WINDOW_HIDDEN);
		if (sdl_window)
		{
			SDL_Renderer* sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (sdl_renderer)
			{
				int framebuffer_width = window_width_initial;
				int framebuffer_height = window_height_initial;
				SDL_Texture* display_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, framebuffer_width, framebuffer_height);

				// @TODO: initialization

				performance_frequency = (double)SDL_GetPerformanceFrequency();

				Uint64 update_timer = SDL_GetPerformanceCounter();
				Uint64 per_second_timer = SDL_GetPerformanceCounter();
				double update_accumulator = 0.0f;
				int update_counter = 0;
				int render_counter = 0;

				SDL_ShowWindow(sdl_window);

				Input input = {};

				while (running)
				{
					SDL_Event ev;
					while (SDL_PollEvent(&ev))
					{
						switch (ev.type)
						{
							case SDL_QUIT:
							{
								running = false;
							} break;

							// @TODO: input, window events
						}
					}


					// Update
					Uint64 update_timer_now = SDL_GetPerformanceCounter();
					update_accumulator += ElapsedTime(update_timer, update_timer_now);
					while (update_accumulator > update_period)
					{
						++update_counter;
						update_accumulator -= update_period;

						// @TODO: update function
						Stub_Update(input, (float)update_period);
					}
					update_timer = update_timer_now;


					// Render
					int output_width;
					int output_height;
					// @TODO: render function
					Uint32* output_pixels = Stub_Render(output_width, output_height);

					if ((output_width != framebuffer_width) || (output_height != framebuffer_height))
					{
						SDL_DestroyTexture(display_texture);
						display_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, output_width, output_height);
						framebuffer_width = output_width;
						framebuffer_height = output_height;
					}

					Uint32* display_pixels;
					int display_pitch;
					SDL_LockTexture(display_texture, nullptr, (void**)&display_pixels, &display_pitch);
					SDL_memcpy(display_pixels, output_pixels, (size_t)output_width * (size_t)output_height * sizeof(Uint32));
					SDL_UnlockTexture(display_texture);

					SDL_RenderCopy(sdl_renderer, display_texture, nullptr, nullptr);

					++render_counter;
					SDL_RenderPresent(sdl_renderer);


					// Per second timing
					Uint64 per_second_timer_now = SDL_GetPerformanceCounter();
					if (ElapsedTime(per_second_timer, per_second_timer_now) >= 1.0)
					{
						char buf[0x100];
						SDL_snprintf(buf, 0x100, "UPS: %d, FPS: %d", update_counter, render_counter);

						update_counter = 0;
						render_counter = 0;
						per_second_timer = per_second_timer_now;
					}
				}
			}
		}
	}

	//system("PAUSE");
	return result;
}

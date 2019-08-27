#include <SDL.h>


static bool running = true;
static Uint64 performance_frequency;


double elapsed_time(Uint64 start, Uint64 end)
{
	return (double)(end - start) / (double)performance_frequency;
}


int main(int argc, char *argv[])
{
	int result = -1;

	int window_width = 1600;
	int window_height = 900;
	double update_frequency = 1.0 / 60.0;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		SDL_Window *sdl_window = SDL_CreateWindow("sdl window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_HIDDEN);
		if (sdl_window)
		{
			SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (sdl_renderer)
			{
				// @TODO: initialize

				performance_frequency = SDL_GetPerformanceFrequency();

				Uint64 update_timer = SDL_GetPerformanceCounter();
				Uint64 per_second_timer = SDL_GetPerformanceCounter();
				double update_accumulator = 0.0f;
				int update_counter = 0;
				int render_counter = 0;

				SDL_ShowWindow(sdl_window);

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
						}
					}
					
					// Update
					Uint64 update_timer_now = SDL_GetPerformanceCounter();
					update_accumulator += elapsed_time(update_timer, update_timer_now);
					while (update_accumulator > update_frequency)
					{
						++update_counter;
						update_accumulator -= update_frequency;

						// @TODO: update
					}
					update_timer = update_timer_now;


					// Render

					// @TODO: render

					SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderClear(sdl_renderer);
					++render_counter;
					SDL_RenderPresent(sdl_renderer);

					Uint64 per_second_timer_now = SDL_GetPerformanceCounter();
					if (elapsed_time(per_second_timer, per_second_timer_now) >= 1.0)
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

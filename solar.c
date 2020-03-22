#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define NUM_PLANETS 6
#define MATH_PI 3.141592
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

struct solar_body
{
	int distance;
	int size;
	double omega; // rads/s
};
typedef struct solar_body solar_body_s;

SDL_Window *window;
SDL_Renderer *renderer;

solar_body_s solar_system[NUM_PLANETS] = {
	{10, 4, MATH_PI},
	{30, 4, MATH_PI / 1},
	{50, 4, MATH_PI / 2},
	{60, 4, MATH_PI / 3},
	{80, 4, MATH_PI / 4},
	{100, 12, MATH_PI / 5}
};

void draw_circle_outlined(int x, int y, int radius)
{
	for (int i = 1; i < 361; i++)
	{
		double rad = ((double)i / 180) * MATH_PI;
		
		double ox = sin(rad) * radius;
		double oy = cos(rad) * radius;

		SDL_RenderDrawPoint(renderer, x + ox, y + oy);
	}
}

void draw_circle_filled(int x, int y, int radius)
{
	for (int i = 1; i < 181; i++)
	{
		double rad = ((double)i / 180) * MATH_PI;
		
		double ox1 = sin(rad) * (double)radius;
		double oy1 = cos(rad) * (double)radius;

		double ox2 = ox1 * -1;
		double oy2 = oy1 * -1;
	
		SDL_RenderDrawLine(renderer, x + ox1, y + oy1, x + ox2, y + oy2);
	}
}
		
int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	int quit = 0;
	SDL_Event eventhdl;

	while (!quit)
	{
		while (SDL_PollEvent(&eventhdl))
		{
			switch (eventhdl.type)
			{
				case SDL_QUIT:
					quit = 1;
			}
		}
		
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		int ticks = SDL_GetTicks();

		for (int i = 0; i < NUM_PLANETS; i++)
		{
			solar_body_s *planet = &solar_system[i];

			double delta = (planet->omega / 1000) * ticks;

			double x = sin(delta) * planet->distance;
			double y = cos(delta) * planet->distance;

			draw_circle_outlined(SCREEN_WIDTH / 2, SCREEN_WIDTH /2, planet->distance);
			draw_circle_filled(SCREEN_WIDTH / 2 + x, SCREEN_WIDTH /2 + y, planet->size);

			//SDL_RenderFillRect(renderer, &screen_rect);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

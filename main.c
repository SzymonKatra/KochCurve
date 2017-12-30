#include <SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define VIEWPORT_STEP 1

int main(int argc, char* args[])
{
	int pointsCount;
	SDL_Point points[100];

	printf("Enter number of points: ");
	scanf("%d", &pointsCount);
	for (int i = 0; i < pointsCount; i++)
	{
		printf("Enter #%d point: ", i + 1);
		scanf("%d %d", &points[i].x, &points[i].y);
	}
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return -1;
	}
	
	printf("SDL_Init success\n");
	
	SDL_Window* window = SDL_CreateWindow("Koch curve", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		return -1;
	}
	
	printf("SDL_CreateWindow success\n");
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
	{
		printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
		return -1;
	}
	
	printf("SDL_CreateRenderer success\n");
	
	SDL_Rect viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.w = SCREEN_WIDTH;
	viewport.h = SCREEN_HEIGHT;
	
	int running = 1;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						running = 0;
					}
					break;
					
				case SDL_QUIT:
					running = 0;
					break;
			}
		}
		
		const Uint8* keyState = SDL_GetKeyboardState(NULL);
		if (keyState[SDL_SCANCODE_LEFT]) viewport.x += VIEWPORT_STEP;
		if (keyState[SDL_SCANCODE_RIGHT]) viewport.x -= VIEWPORT_STEP;
		if (keyState[SDL_SCANCODE_UP]) viewport.y += VIEWPORT_STEP;
		if (keyState[SDL_SCANCODE_DOWN]) viewport.y -= VIEWPORT_STEP;
		
		SDL_RenderSetViewport(renderer, &viewport);
		
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLines(renderer, points, pointsCount);
		
		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	
	SDL_Quit();
	
	return 0;
}
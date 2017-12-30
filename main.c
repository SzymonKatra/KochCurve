#include <SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window* window = SDL_CreateWindow("Koch curve", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	
	SDL_Delay(2000);
	
	SDL_FreeSurface(surface);
	surface = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	
	SDL_Quit();
	
	return 0;
}
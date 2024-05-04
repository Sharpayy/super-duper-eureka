#pragma once
#include <SDL.h>
#include <functional>

#ifdef main
#undef main
#endif

class WindowSDL
{
public:
	SDL_Window* win;
	//SDL_Renderer* r;

	SDL_GLContext glContext;
	SDL_Event e;
	
	std::function<void(SDL_Event*)> customEventDispatch;

	WindowSDL(const char* title, int x, int y);
	WindowSDL(const char* title, int x, int y, Uint32 flg);

	void glCreateContext();
	void handleEvents();

	void swap(); 

};

void windowEventMinDispatch(SDL_Event* e);


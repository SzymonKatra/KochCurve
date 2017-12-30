#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CAMERA_STEP 0.1
#define CAMERA_SCALE_STEP 0.01

int testasm(double a, double b);

typedef struct
{
	float x;
	float y;
	float originX;
	float originY;
	float scale;
} cameraState_t;

int main(int argc, char* args[])
{
	al_init();
	al_init_primitives_addon();
	
	al_install_keyboard();
	al_install_mouse();
	
	ALLEGRO_DISPLAY* display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	ALLEGRO_COLOR white = al_map_rgba(255, 255, 255, 255);
	ALLEGRO_COLOR black = al_map_rgba(0, 0, 0, 255);
	
	cameraState_t camera;
	camera.x = camera.y = 0;
	camera.scale = 1;
	
	int cameraMoveBeginX, cameraMoveBeginY;
	int mouseMoveBeginX, mouseMoveBeginY;
	int moving = 0;
	
	while (1)
	{
		ALLEGRO_EVENT event;
		while (al_get_next_event(queue, &event))
		{
			if (event.type == ALLEGRO_EVENT_MOUSE_AXES)
			{
				if (event.mouse.dz != 0 && !moving)
				{
					camera.x = (event.mouse.x - SCREEN_WIDTH / 2) / camera.scale;
					camera.y = (event.mouse.y - SCREEN_HEIGHT / 2) / camera.scale;
					
					double delta = event.mouse.dz;
					delta /= 10;
					camera.scale += delta;
					
					if (camera.scale < 0.1) camera.scale = 0.1;
					
					
				}
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
			{
				cameraMoveBeginX = camera.x;
				cameraMoveBeginY = camera.y;
				mouseMoveBeginX = event.mouse.x;
				mouseMoveBeginY = event.mouse.y;
				moving = 1;
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
			{
				double deltaX = mouseMoveBeginX - event.mouse.x;
				double deltaY = mouseMoveBeginY - event.mouse.y;
				
				camera.x = cameraMoveBeginX + deltaX;
				camera.y = cameraMoveBeginY + deltaY;
				moving = 0;
			}
		}
		
		ALLEGRO_KEYBOARD_STATE keyboardState;
		ALLEGRO_MOUSE_STATE mouseState;
		al_get_keyboard_state(&keyboardState);
		al_get_mouse_state(&mouseState);
		if (moving)
		{
			double deltaX = mouseMoveBeginX - mouseState.x;
			double deltaY = mouseMoveBeginY - mouseState.y;
				
			camera.x = cameraMoveBeginX + deltaX;
			camera.y = cameraMoveBeginY + deltaY;
		}
		
		ALLEGRO_TRANSFORM cameraMatrix;
		al_identity_transform(&cameraMatrix);
		
		al_translate_transform(&cameraMatrix, -camera.originX, -camera.originY);
		al_scale_transform(&cameraMatrix, camera.scale, camera.scale);
		al_translate_transform(&cameraMatrix, camera.originX, camera.originY);
		al_translate_transform(&cameraMatrix, -camera.x, -camera.y);		
		
		
		al_use_transform(&cameraMatrix);
		
		al_clear_to_color(white);
	
		al_draw_line(0, 0, 300, 600, black, 1 / camera.scale);
		al_draw_line(300, 600, -300, 600, black, 1 / camera.scale);
		al_draw_line(-300, 600, 0, 0, black, 1 / camera.scale);
		
		al_wait_for_vsync();
		al_flip_display();
	}
	
	al_destroy_event_queue(queue);
	al_destroy_display(display);
	
	return 0;
}
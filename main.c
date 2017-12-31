#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CAMERA_STEP 0.1
#define CAMERA_SCALE_STEP 0.01

typedef struct
{
	float x;
	float y;
	float originX;
	float originY;
	float scale;
} cameraState_t;

typedef struct
{
	float x;
	float y;
} point_t;

void koch(point_t* input, point_t* output, int count);

void computeCamera(const cameraState_t* camera, ALLEGRO_TRANSFORM* matrix)
{
	al_identity_transform(matrix);
		
	al_translate_transform(matrix, -camera->originX, -camera->originY);
	al_scale_transform(matrix, camera->scale, camera->scale);
	al_translate_transform(matrix, camera->originX, camera->originY);
		
	al_translate_transform(matrix, -camera->x, -camera->y);
}

int main(int argc, char* args[])
{
	al_init();
	al_init_primitives_addon();
	
	al_install_keyboard();
	al_install_mouse();
	
	ALLEGRO_DISPLAY* display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	ALLEGRO_COLOR white = al_map_rgba(255, 255, 255, 255);
	ALLEGRO_COLOR black = al_map_rgba(0, 0, 0, 255);
	
	cameraState_t camera;
	camera.x = camera.y = camera.originX = camera.originY = 0;
	camera.scale = 1;
	
	int cameraMoveBeginX, cameraMoveBeginY;
	int mouseMoveBeginX, mouseMoveBeginY;
	int moving = 0;
	int pointsCount = 4;
	point_t* points = malloc(sizeof(point_t) * pointsCount);
	points[0].x = -100;
	points[0].y = 0;
	points[1].x = 0;
	points[1].y = 200;
	points[2].x = 100;
	points[2].y = 0;
	points[3] = points[0];
	
	while (1)
	{
		ALLEGRO_EVENT event;
		while (al_get_next_event(queue, &event))
		{
			if (event.type == ALLEGRO_EVENT_MOUSE_AXES)
			{
				if (event.mouse.dz != 0 && !moving)
				{	
					ALLEGRO_TRANSFORM transform;
					
					computeCamera(&camera, &transform);
					al_invert_transform(&transform);
					
					float worldX = event.mouse.x;
					float worldY = event.mouse.y;
					al_transform_coordinates(&transform, &worldX, &worldY);
					
					camera.originX = worldX;
					camera.originY = worldY;
					
					float delta = event.mouse.dz;
					delta /= 10;
					camera.scale += delta;
					
					if (camera.scale < 0.1) camera.scale = 0.1;
					
					computeCamera(&camera, &transform);
					al_invert_transform(&transform);
					
					float newWorldX = event.mouse.x;
					float newWorldY = event.mouse.y;
					al_transform_coordinates(&transform, &newWorldX, &newWorldY);
					
					camera.x += (worldX - newWorldX) * camera.scale;
					camera.y += (worldY - newWorldY) * camera.scale;
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
				float deltaX = mouseMoveBeginX - event.mouse.x;
				float deltaY = mouseMoveBeginY - event.mouse.y;
				
				camera.x = cameraMoveBeginX + deltaX;
				camera.y = cameraMoveBeginY + deltaY;
				moving = 0;
			}
			else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				if (event.keyboard.keycode == ALLEGRO_KEY_SPACE)
				{
					int newPointsCount = (pointsCount - 1) * 4 + 1;
					point_t* newPoints = malloc(sizeof(point_t) * newPointsCount);
					koch(points, newPoints, pointsCount);
					
					free(points);
					points = newPoints;
					pointsCount = newPointsCount;
				}
			}
		}
		
		ALLEGRO_KEYBOARD_STATE keyboardState;
		ALLEGRO_MOUSE_STATE mouseState;
		al_get_keyboard_state(&keyboardState);
		al_get_mouse_state(&mouseState);
		if (moving)
		{
			float deltaX = mouseMoveBeginX - mouseState.x;
			float deltaY = mouseMoveBeginY - mouseState.y;
				
			camera.x = cameraMoveBeginX + deltaX;
			camera.y = cameraMoveBeginY + deltaY;
		}
		
		ALLEGRO_TRANSFORM cameraMatrix;
		computeCamera(&camera, &cameraMatrix);
		al_use_transform(&cameraMatrix);
		
		al_clear_to_color(white);
		
		float thickness = 1 / camera.scale;
		for (int i = 0; i < pointsCount - 1; i++)
		{
			/*float x1 = points[i].x;
			float y1 = points[i].y;
			float x2 = points[i + 1].x;
			float y2 = points[i + 1].y;
			al_transform_coordinates(&cameraMatrix, &x1, &y1);
			al_transform_coordinates(&cameraMatrix, &x2, &y2);	
			
			// check bounds
			int x1Out = x1 < 0 || x1 > SCREEN_WIDTH;
			int y1Out = y1 < 0 || y1 > SCREEN_HEIGHT;
			int x2Out = x2 < 0 || x2 > SCREEN_WIDTH;
			int y2Out = y2 < 0 || y2 > SCREEN_HEIGHT;
			
			if (x1Out && y1Out && x2Out && y2Out) continue; // don't draw things for sure not covered by camera*/
			
			al_draw_line(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, black, thickness);
		}
		
		al_wait_for_vsync();
		al_flip_display();
	}
	
	al_destroy_event_queue(queue);
	al_destroy_display(display);
	
	return 0;
}
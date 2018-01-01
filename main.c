#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define F_EPSILON 0.0001

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

float crossProduct(point_t a, point_t b)
{
	return a.x * b.y - a.y * b.x;
}
// a1 - start of segment A
// a2 - end of segment A
// b1 - start of segment B
// b2 - start of segment B
int segmentsIntersect(point_t a1, point_t a2, point_t b1, point_t b2)
{
	// https://stackoverflow.com/questions/563198/whats-the-most-efficent-way-to-calculate-where-two-line-segments-intersect/565282#565282
	
	point_t p = a1;
	point_t r; // a2 - a1
	r.x = a2.x - a1.x;
	r.y = a2.y - a1.y;
	
	point_t q = b1;
	point_t s; // b2 - b1
	s.x = b2.x - b1.x;
	s.y = b2.y - b1.y;
	
	point_t m; // q - p
	m.x = q.x - p.x;
	m.y = q.y - p.y;
	
	float cross_rs = crossProduct(r, s);
	float cross_ms = crossProduct(m, s);
	float cross_mr = crossProduct(m, r);
	
	if (fabsf(cross_rs) < F_EPSILON)
	{
		return fabsf(cross_mr) < F_EPSILON;
	}
	else
	{
		float t = cross_ms / cross_rs;
		float u = cross_mr / cross_rs;
		
		return t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f;
	}
}
// a - top-left corner of rectangle
// b - bottom-right corner of rectangle
// p - point to check
int isPointInsideRectangle(point_t a, point_t b, point_t p)
{
	return p.x >= a.x && p.x <= b.x && p.y >= a.y && p.y <= b.y;
}
int isInsideCamera(point_t cameraBounds[4], point_t a, point_t b)
{	
	return (isPointInsideRectangle(cameraBounds[0], cameraBounds[2], a) ||
			isPointInsideRectangle(cameraBounds[0], cameraBounds[2], b) ||
			segmentsIntersect(cameraBounds[0], cameraBounds[1], a, b) ||
			segmentsIntersect(cameraBounds[1], cameraBounds[2], a, b) ||
			segmentsIntersect(cameraBounds[2], cameraBounds[3], a, b) ||
			segmentsIntersect(cameraBounds[3], cameraBounds[0], a, b));
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
	
	point_t screenBounds[4];
	screenBounds[0].x = 0;
	screenBounds[0].y = 0;
	screenBounds[1].x = SCREEN_WIDTH;
	screenBounds[1].y = 0;
	screenBounds[2].x = SCREEN_WIDTH;
	screenBounds[2].y = SCREEN_HEIGHT;
	screenBounds[3].x = 0;
	screenBounds[3].y = SCREEN_HEIGHT;
	
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
					if (camera.scale < 10) delta /= 5;
					else if (camera.scale > 500) delta *= 25;
					else if (camera.scale > 70) delta *= 5;
					camera.scale += delta;
					
					if (camera.scale < 0.1) camera.scale = 0.1;
					
					printf("%lf\n", camera.scale);
					
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
		
		for (int i = 0; i < pointsCount - 1; i++)
		{
			point_t a = points[i];
			point_t b = points[i + 1];
			al_transform_coordinates(&cameraMatrix, &a.x, &a.y);
			al_transform_coordinates(&cameraMatrix, &b.x, &b.y);	
			
			if (isInsideCamera(screenBounds, a, b))
			{
				al_draw_line(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, black, 1.0f / camera.scale);
			}
		}
		
		al_wait_for_vsync();
		al_flip_display();
	}
	
	al_destroy_event_queue(queue);
	al_destroy_display(display);
	
	return 0;
}
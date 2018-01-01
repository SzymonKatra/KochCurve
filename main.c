#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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

void koch(point_t* input, point_t* output, int count); // koch.asm

void initAll();
void freeAll();
void zoomAt(int x, int y, int value);
void startMoving(int mouseX, int mouseY);
void move(int mouseX, int mouseY);
void endMoving(int mouseX, int mouseY);
void makeKochStep();

void initCamera(cameraState_t* camera);
void computeCamera(const cameraState_t* camera, ALLEGRO_TRANSFORM* matrix);

float crossProduct(point_t a, point_t b);
int segmentsIntersect(point_t a1, point_t a2, point_t b1, point_t b2);
int isPointInsideRectangle(point_t a, point_t b, point_t p);
int isInsideBounds(point_t bounds[4], point_t a, point_t b);

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_COLOR white, black;
ALLEGRO_FONT* font;
cameraState_t camera, hudCamera;
point_t cameraMoveBegin, mouseMoveBegin;
point_t* points;
int moving, pointsCount, kochStep;
point_t screenBounds[4];
int running;

int main(int argc, char* args[])
{
	initAll();
	
	while (running)
	{
		ALLEGRO_EVENT event;
		while (al_get_next_event(queue, &event))
		{
			switch (event.type)
			{
				case ALLEGRO_EVENT_MOUSE_AXES: zoomAt(event.mouse.x, event.mouse.y, event.mouse.dz); break;
				case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: startMoving(event.mouse.x, event.mouse.y); break;
				case ALLEGRO_EVENT_MOUSE_BUTTON_UP: endMoving(event.mouse.x, event.mouse.y); break;
				case ALLEGRO_EVENT_KEY_DOWN:
					switch (event.keyboard.keycode)
					{
						case ALLEGRO_KEY_SPACE: makeKochStep(); break;
						case ALLEGRO_KEY_ESCAPE: running = 0; break;
					}
					break;
			}
		}
			
		if (moving)
		{
			ALLEGRO_MOUSE_STATE mouseState;
			al_get_mouse_state(&mouseState);
			move(mouseState.x, mouseState.y);
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
			
			if (isInsideBounds(screenBounds, a, b))
			{
				al_draw_line(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, black, 1.0f / camera.scale);
			}
		}
		
		computeCamera(&hudCamera, &cameraMatrix);
		al_use_transform(&cameraMatrix);
		
		char buffer[128];
		sprintf(buffer, "Step: %d", kochStep);
		al_draw_text(font, black, 5, 5, ALLEGRO_ALIGN_LEFT, buffer);
		sprintf(buffer, "Zoom: %.1f", camera.scale);
		al_draw_text(font, black, 5, 25, ALLEGRO_ALIGN_LEFT, buffer);
		
		al_wait_for_vsync();
		al_flip_display();
	}
	
	freeAll();
	
	return 0;
}

void initAll()
{
	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	
	al_install_keyboard();
	al_install_mouse();
	
	display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	white = al_map_rgba(255, 255, 255, 255);
	black = al_map_rgba(0, 0, 0, 255);
	font = al_load_ttf_font("consola.ttf", 20, 0);
	
	initCamera(&camera);
	initCamera(&hudCamera);
	
	moving = 0;
	kochStep = 0;
	pointsCount = 4;
	points = malloc(sizeof(point_t) * pointsCount);
	points[0].x = -100;
	points[0].y = 0;
	points[1].x = 0;
	points[1].y = 200;
	points[2].x = 100;
	points[2].y = 0;
	points[3] = points[0];
	
	screenBounds[0].x = 0;
	screenBounds[0].y = 0;
	screenBounds[1].x = SCREEN_WIDTH;
	screenBounds[1].y = 0;
	screenBounds[2].x = SCREEN_WIDTH;
	screenBounds[2].y = SCREEN_HEIGHT;
	screenBounds[3].x = 0;
	screenBounds[3].y = SCREEN_HEIGHT;
	
	running = 1;
}

void freeAll()
{
	free(points);
	al_destroy_font(font);
	al_destroy_event_queue(queue);
	al_destroy_display(display);
}

void zoomAt(int x, int y, int value)
{
	if (value == 0 || moving) return;
	
	ALLEGRO_TRANSFORM transform;
					
	computeCamera(&camera, &transform);
	al_invert_transform(&transform);
					
	float worldX = x;
	float worldY = y;
	al_transform_coordinates(&transform, &worldX, &worldY);
					
	camera.originX = worldX;
	camera.originY = worldY;
					
	float delta = value;
	if (camera.scale < 9.9f) delta /= 5;
	else if (camera.scale >= 500) delta *= 25;
	else if (camera.scale >= 70) delta *= 5;
	camera.scale += delta;
					
	if (camera.scale < 0.1f) camera.scale = 0.1f;
					
	computeCamera(&camera, &transform);
	al_invert_transform(&transform);
					
	float newWorldX = x;
	float newWorldY = y;
	al_transform_coordinates(&transform, &newWorldX, &newWorldY);
					
	camera.x += (worldX - newWorldX) * camera.scale;
	camera.y += (worldY - newWorldY) * camera.scale;
}

void startMoving(int mouseX, int mouseY)
{
	cameraMoveBegin.x = camera.x;
	cameraMoveBegin.y = camera.y;
	mouseMoveBegin.x = mouseX;
	mouseMoveBegin.y = mouseY;
	moving = 1;
}
void move(int mouseX, int mouseY)
{
	float deltaX = mouseMoveBegin.x - mouseX;
	float deltaY = mouseMoveBegin.y - mouseY;
				
	camera.x = cameraMoveBegin.x + deltaX;
	camera.y = cameraMoveBegin.y + deltaY;
}
void endMoving(int mouseX, int mouseY)
{
	move(mouseX, mouseY);
	moving = 0;
}
void makeKochStep()
{
	int newPointsCount = (pointsCount - 1) * 4 + 1;
	point_t* newPoints = malloc(sizeof(point_t) * newPointsCount);
	koch(points, newPoints, pointsCount);
					
	free(points);
	points = newPoints;
	pointsCount = newPointsCount;
					
	kochStep++;
}

void initCamera(cameraState_t* camera)
{
	camera->x = camera->y = camera->originX = camera->originY = 0;
	camera->scale = 1;
}

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
		
		return t >= 0 && t <= 1 && u >= 0 && u <= 1;
	}
}

// a - top-left corner of rectangle
// b - bottom-right corner of rectangle
// p - point to check
int isPointInsideRectangle(point_t a, point_t b, point_t p)
{
	return p.x >= a.x && p.x <= b.x && p.y >= a.y && p.y <= b.y;
}

// a - start of the segment
// b - end of the segment
int isInsideBounds(point_t bounds[4], point_t a, point_t b)
{	
	return (isPointInsideRectangle(bounds[0], bounds[2], a) ||
			isPointInsideRectangle(bounds[0], bounds[2], b) ||
			segmentsIntersect(bounds[0], bounds[1], a, b) ||
			segmentsIntersect(bounds[1], bounds[2], a, b) ||
			segmentsIntersect(bounds[2], bounds[3], a, b) ||
			segmentsIntersect(bounds[3], bounds[0], a, b));
}


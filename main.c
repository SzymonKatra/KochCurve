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

void koch(point_t* output, point_t* input, int count); // koch.asm

void initAll();
void freeAll();
void restart();
void processEvents();
void drawHud();
void zoomAt(int mouseX, int mouseY, int value);
void startMoving(int mouseX, int mouseY);
void move(int mouseX, int mouseY);
void endMoving(int mouseX, int mouseY);
void makeKochStep();
void undoKochStep();
void appendPoint(int x, int y);
void applyDefaultCurve();

void initCamera(cameraState_t* camera);
void computeCamera(const cameraState_t* camera, ALLEGRO_TRANSFORM* matrix);
point_t mouseToWorld(int mouseX, int mouseY);

float crossProduct(point_t a, point_t b);
int segmentsIntersect(point_t a1, point_t a2, point_t b1, point_t b2);
int isPointInsideRectangle(point_t a, point_t b, point_t p);
int isInsideBounds(point_t bounds[4], point_t a, point_t b);

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_COLOR white, black, red;
ALLEGRO_FONT* font;
cameraState_t camera, hudCamera;
point_t cameraMoveBegin, mouseMoveBegin;
point_t* points;
int moving, pointsCount, kochStep;
point_t screenBounds[4];
int running;
int drawing;

int main(int argc, char** argv)
{
	initAll();
	
	while (running)
	{
		processEvents();
		
		ALLEGRO_MOUSE_STATE mouseState;
		al_get_mouse_state(&mouseState);
		if (moving) move(mouseState.x, mouseState.y);
		
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
		
		if (drawing && pointsCount > 0)
		{
			point_t target = mouseToWorld(mouseState.x, mouseState.y);
			al_draw_line(points[pointsCount - 1].x, points[pointsCount - 1].y, target.x, target.y, red, 1.0f / camera.scale);
		}
		
		computeCamera(&hudCamera, &cameraMatrix);
		al_use_transform(&cameraMatrix);
		
		drawHud();
		
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
	al_register_event_source(queue, al_get_display_event_source(display));
	white = al_map_rgba(255, 255, 255, 255);
	black = al_map_rgba(0, 0, 0, 255);
	red = al_map_rgba(255, 0, 0, 255);
	font = al_load_ttf_font("consola.ttf", 20, 0);
	
	initCamera(&camera);
	initCamera(&hudCamera);
	
	moving = 0;
	kochStep = 0;
	pointsCount = 0;
	points = (point_t*)malloc(0);
	
	screenBounds[0].x = 0;
	screenBounds[0].y = 0;
	screenBounds[1].x = SCREEN_WIDTH;
	screenBounds[1].y = 0;
	screenBounds[2].x = SCREEN_WIDTH;
	screenBounds[2].y = SCREEN_HEIGHT;
	screenBounds[3].x = 0;
	screenBounds[3].y = SCREEN_HEIGHT;
	
	drawing = 1;
	running = 1;
}

void freeAll()
{
	free(points);
	points = NULL;
	al_destroy_font(font);
	al_destroy_event_queue(queue);
	al_destroy_display(display);
}

void restart()
{
	initCamera(&camera);
	
	moving = 0;
	kochStep = 0;
	pointsCount = 0;
	points = (point_t*)realloc(points, 0);
	drawing = 1;
}

void processEvents()
{
	ALLEGRO_EVENT event;
	while (al_get_next_event(queue, &event))
	{
		switch (event.type)
		{
			case ALLEGRO_EVENT_MOUSE_AXES: zoomAt(event.mouse.x, event.mouse.y, event.mouse.dz); break;
				
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				switch (event.mouse.button)
				{
					case 1: startMoving(event.mouse.x, event.mouse.y); break;
					case 2: if (drawing)
							{
								point_t world = mouseToWorld(event.mouse.x, event.mouse.y);
								appendPoint(world.x, world.y);
							}
							break;
				}	
				break;
					
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				switch (event.mouse.button)
				{
					case 1: endMoving(event.mouse.x, event.mouse.y); break;
				}	
				break;
				
			case ALLEGRO_EVENT_KEY_DOWN:
				switch (event.keyboard.keycode)
				{
					case ALLEGRO_KEY_SPACE:
						if (!drawing)
						{
							makeKochStep();
						}
						else
						{
							if (pointsCount > 1)
								appendPoint(points[0].x, points[0].y);
							else applyDefaultCurve();
							drawing = 0;
						}
						break;
						
					case ALLEGRO_KEY_BACKSPACE: if (!drawing) undoKochStep(); break;
					
					case ALLEGRO_KEY_R: restart(); break;
						
					case ALLEGRO_KEY_ESCAPE: running = 0; break;
				}
				break;
				
			case ALLEGRO_EVENT_DISPLAY_CLOSE: running = 0; break;
		}
	}
}

void drawHud()
{
	if (!drawing)
	{
		char buffer[128];
		sprintf(buffer, "Step: %d", kochStep);
		al_draw_text(font, black, 5, 5, ALLEGRO_ALIGN_LEFT, buffer);
		sprintf(buffer, "Zoom: %.1f", camera.scale);
		al_draw_text(font, black, 5, 25, ALLEGRO_ALIGN_LEFT, buffer);
	}
	else 
	{
		if (pointsCount == 0)
		{
			al_draw_text(font, black, 5, 5, ALLEGRO_ALIGN_LEFT,  "ESC          - exit");
			al_draw_text(font, black, 5, 25, ALLEGRO_ALIGN_LEFT, "R            - restart");
			al_draw_text(font, black, 5, 45, ALLEGRO_ALIGN_LEFT, "SPACE        - make step of koch curve");
			al_draw_text(font, black, 5, 65, ALLEGRO_ALIGN_LEFT, "BACKSPACE    - undo step of koch curve");
			al_draw_text(font, black, 5, 85, ALLEGRO_ALIGN_LEFT, "Drag & drop with left mouse button");
			al_draw_text(font, black, 5, 105, ALLEGRO_ALIGN_LEFT, "             - change position of camera");
			al_draw_text(font, black, 5, 125, ALLEGRO_ALIGN_LEFT, "Mouse scroll - change zoom");
			al_draw_text(font, red, SCREEN_WIDTH / 2, 205, ALLEGRO_ALIGN_CENTER, "To begin, click right mouse button and start drawing");
			al_draw_text(font, red, SCREEN_WIDTH / 2, 225, ALLEGRO_ALIGN_CENTER, "initial curve or press SPACE to draw default one.");
		}
		else
		{
			al_draw_text(font, black, 5, 5, ALLEGRO_ALIGN_LEFT, "Press SPACE to finish drawing");
		}
	}
}

void zoomAt(int mouseX, int mouseY, int value)
{
	if (value == 0 || moving) return;
	
	point_t world = mouseToWorld(mouseX, mouseY);
					
	camera.originX = world.x;
	camera.originY = world.y;
					
	float delta = value;
	if (camera.scale < 9.9f) delta /= 5;
	else if (camera.scale >= 500) delta *= 25;
	else if (camera.scale >= 70) delta *= 5;
	camera.scale += delta;
					
	if (camera.scale < 0.1f) camera.scale = 0.1f;
					
	point_t newWorld = mouseToWorld(mouseX, mouseY);
					
	camera.x += (world.x - newWorld.x) * camera.scale;
	camera.y += (world.y - newWorld.y) * camera.scale;
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
	koch(newPoints, points, pointsCount);
					
	free(points);
	points = newPoints;
	pointsCount = newPointsCount;
					
	kochStep++;
}

void undoKochStep()
{
	if (kochStep == 0) return;
	
	int newPointsCount = (pointsCount - 1) / 4 + 1;
	point_t* newPoints = malloc(sizeof(point_t) * newPointsCount);

	for (int i = 0; i < newPointsCount; i++)
	{
		newPoints[i] = points[i * 4];
	}
	
	free(points);
	points = newPoints;
	pointsCount = newPointsCount;
	
	kochStep--;
}

void appendPoint(int x, int y)
{
	pointsCount++;
	points = (point_t*)realloc(points, sizeof(point_t) * pointsCount);
	points[pointsCount - 1].x = x;
	points[pointsCount - 1].y = y;
}

void applyDefaultCurve()
{
	pointsCount = 4;
	points = (point_t*)realloc(points, sizeof(point_t) * pointsCount);
	points[0].x = 170;
	points[0].y = 150;
	points[1].x = 470;
	points[1].y = 150;
	points[2].x = 320;
	points[2].y = 410;
	points[3] = points[0];
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

point_t mouseToWorld(int mouseX, int mouseY)
{
	ALLEGRO_TRANSFORM inverseCameraMatrix;
	computeCamera(&camera, &inverseCameraMatrix);
	al_invert_transform(&inverseCameraMatrix);
			
	point_t world;
	world.x = mouseX;
	world.y = mouseY;
	al_transform_coordinates(&inverseCameraMatrix, &world.x, &world.y);
	
	return world;
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


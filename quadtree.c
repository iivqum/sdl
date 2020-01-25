//gcc -o quadtree -Iinclude/SDL2 quadtree.c -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#define window_width 512
#define window_height 512

#define qt_max_points 1
#define max_points 10000

SDL_Renderer *renderer;
SDL_Window *window;

typedef struct{
	int x, y;
} point;

typedef struct quadtree_node {
	int x, y;
	int w, h;
	
	point **points;
	int pcount; //how many points
	
	struct quadtree_node *sub;
} quadtree_node;

point *points[max_points] = {NULL};

#define point_in_aabb(x1, y1, x2, y2, x3, y3) (x1 > x2 && x1 < x3 && y1 > y2 && y1 < y3)

point *point_add(int x, int y)
{
	for (int i = 0; i < max_points; i++){
		if (points[i] == NULL){
			point *pt = malloc(sizeof(point));

			if (pt == NULL){
				perror("could not alloc point");
				exit(EXIT_FAILURE);
			}
			
			pt->x = x; pt->y = y;
			points[i] = pt;
		
			return pt;
		}
	}
	return NULL;
}

int quadtree_init(quadtree_node *nd, int x, int y, int w, int h)
{
	if (nd == NULL){
		return 0;
	}
	nd->x = x; nd->y = y; nd->w = w; nd->h = h;
	nd->pcount = 0, nd->sub = NULL; nd->points = NULL;
	return 1;
}

int quadtree_destroy(quadtree_node *nd)
{
	if (nd->sub != NULL){
		for (int i = 0; i < 4; i++){
			quadtree_destroy(&nd->sub[i]);
		}
	}
	if (nd->points != NULL){
		free(nd->points);
	}
	free(nd);
	return 1;
}

int quadtree_insert(quadtree_node *nd, point *pt)
{
	//if point is outside this node, dont do anything
	if (!point_in_aabb(pt->x, pt->y, nd->x, nd->y, nd->x + nd->w, nd->y + nd->h)) {	
		return 0;
	}	
	//check if node has no children
	if (nd->sub == NULL){
		//should node split?
		if (nd->pcount == qt_max_points){
			//split
			nd->sub = malloc(4 * sizeof(quadtree_node));
			
			if (nd->sub == NULL){
				perror("could not alloc nodes");
				exit(EXIT_FAILURE);
			}
			
			int halfwidth = nd->w >> 1; int halfheight = nd->h >> 1;
			int x = nd->x + halfwidth; int y = nd->y + halfheight;
			
			quadtree_init(nd->sub, nd->x, nd->y, halfwidth, halfheight);
			quadtree_init(nd->sub + 1, x, nd->y, halfwidth, halfheight);
			quadtree_init(nd->sub + 2, nd->x, y, halfwidth, halfheight);
			quadtree_init(nd->sub + 3, x, y, halfwidth, halfheight);
			
			//push this nodes points into children
			for (int i = 0; i < qt_max_points; i++){
				for (int j = 0; j < 4; j++){
					if (quadtree_insert(nd->sub + j, nd->points[i])){
						break;
					}
				}
			}
			
			free(nd->points);
			nd->points = NULL;
			
			goto jmp;
		} else {
			nd->pcount++;
			nd->points = realloc(nd->points, nd->pcount * sizeof(point*));	
			
			if (nd->points == NULL){
				perror("could not realloc points");
				exit(EXIT_FAILURE);
			}
			
			nd->points[nd->pcount - 1] = pt;
			
			return 1;
		}
	}
	jmp:
	//push point into children
	for (int i = 0; i < 4; i++){
		if (quadtree_insert(nd->sub + i, pt)){
			return 1;
		}
	}

	return 0;
}

int outlined_rect(int x, int y, int w, int h)
{
	SDL_RenderDrawLine(renderer, x, y, x + w, y);//top 
	SDL_RenderDrawLine(renderer, x, y + h, x + w, y + h);//bottom
	SDL_RenderDrawLine(renderer, x, y, x, y + h);//left
	SDL_RenderDrawLine(renderer, x + w, y, x + w, y + h);//right
	
	return 1;
}

int draw_quadtree(quadtree_node *nd)
{
	if (nd->sub != NULL){
		draw_quadtree(nd->sub);
		draw_quadtree(nd->sub + 1);
		draw_quadtree(nd->sub + 2);
		draw_quadtree(nd->sub + 3);
	}
	
	outlined_rect(nd->x, nd->y, nd->w, nd->h);
	
	return 1;
}

int main(int argc, char *argv[])
{
	quadtree_node root;
	
	if (!quadtree_init(&root, 0, 0, 512, 512)){
		
		return 0;
	}
	
	for (int i = 0; i < max_points; i++){
		if (points[i] != NULL){
			quadtree_insert(&root, points[i]);
		}
	}
	
	SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow("quadtree",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,window_width,window_height,SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	SDL_Event event;
	
	int mouse_held = 0;
	
	for (;;){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_MOUSEMOTION:
					if (mouse_held){
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);
						SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
						
						point *p = point_add(event.button.x, event.button.y);
						
						if (p == NULL){
							perror("too many points");
							goto quit;
						}
						
						quadtree_insert(&root, p);
						draw_quadtree(&root);
						
						for (int i = 0; i < max_points; i++){
							if (points[i] != NULL){
								SDL_RenderDrawPoint(renderer, points[i]->x, points[i]->y);
							}
						}						
					}
					break;
				case SDL_MOUSEBUTTONUP:
					mouse_held = 0;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT){
						mouse_held = 1;
					}
					break;
				case SDL_QUIT:
					goto quit;
			}
		}
		
		SDL_RenderPresent(renderer);
	}
	quit:
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}

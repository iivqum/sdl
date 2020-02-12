/*
gcc -o sand -Iinclude/SDL2 sand.c -Llib -lmingw32 -lSDL2main -lSDL2
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>

#define window_width 512
#define window_height 512

#define ui_btn_cnt 7

/* define up to 7 cell types 
8th bit reserved for update flag */
#define C_EMPTY 2^0
#define C_SOLID 2^1
#define C_FLOW 2^2

/* changing c syntax bad idea */
#define btn_init(x, y, w, h) {x, y, x + w, y + h}

/* assumes x < x2 and y < y2 */
typedef struct{
	/* min */
	int16_t x, y;
	/* max */
	int16_t x2, y2;	
} ui_rct;

SDL_Window *window;
SDL_Renderer *renderer;

ui_rct ui_btn[ui_btn_cnt] = {
btn_init(16, 16, 32, 16),
btn_init(16, 36, 32, 16),
btn_init(16, 56, 32, 16),
btn_init(16, 76, 32, 16),
btn_init(16, 96, 32, 16),
btn_init(16, 116, 32, 16),
btn_init(16, 136, 32, 16)
};

/* initialize cells to empty */
int8_t cells[window_width][window_height] = {C_EMPTY};
int8_t c_select = C_EMPTY;

ui_rct *btn_select;

/* check all 7 buttons, return which is selected */
int16_t btn_chk(int16_t x, int16_t y, ui_rct **out)
{	
	ui_rct *rct;
	for (int i = 0; i < ui_btn_cnt; i++){
		rct = &ui_btn[i];
		/* rect vs point */
		if (x > rct->x && x < rct->x2 && y > rct->y && y < rct->y2){
			if (out != NULL){
				*out = rct;
			}
			return i;
		}
	}
	return -1;
}

void mouse_clk(int16_t x, int16_t y)
{
	/* outside of the screen for some reason */
	if (x > window_width || y > window_height){
		return;
	}
	/* check buttons */
	switch(btn_chk(x, y, NULL)){
		case 0: c_select = C_EMPTY; return;
		case 1: c_select = C_SOLID; return;
		case 2: c_select = C_FLOW; return;
		case 3: return;
		case 4:	return;
		case 5:	return;
		case 6:	return;
	}
	/* no button was pressed, change cell state */
	/* screen and cells is 1:1 so we dont need to scale */
	cells[x][y] = c_select;
}

/* called every frame */
void mouse_upt(int16_t x, int16_t y)
{
	ui_rct *rct;
	btn_chk(x, y, &rct);
	if (rct == NULL){
		btn_select = NULL;
		return;
	}
	btn_select = rct;
}

/* update cells */
void upt_cell_st()
{
	
}

/* state update */
void update_st()
{
	static bool m_prv = false;
	
	uint32_t mx,my;
	uint32_t flags = SDL_GetMouseState(&mx, &my);	
	
	/* update mouse pressed state */
	if ((flags & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_prv == false){
		mouse_clk(mx, my);
		printf("click, type %d\n", c_select);
		m_prv = true;
	} else if (!(flags & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_prv == true){
		m_prv = false;
	}
	
	mouse_upt(mx, my);
	upt_cell_st();
}

void outlined_rct(int16_t x, int16_t y, int16_t x2, int16_t y2)
{
	SDL_RenderDrawLine(renderer, x, y, x2, y);
	SDL_RenderDrawLine(renderer, x, y, x, y2);
	SDL_RenderDrawLine(renderer, x2, y, x2, y2);
	SDL_RenderDrawLine(renderer, x, y2, x2, y2);
}

void draw_btn()
{
	ui_rct *rct;
	for (int i = 0; i < ui_btn_cnt; i++){
		rct = &ui_btn[i];
		for (int y = rct->y; y <= rct->y2; y++){
			SDL_RenderDrawLine(renderer, rct->x, y, rct->x2, y);
		}
		if (btn_select == rct){
			outlined_rct(rct->x - 4, rct->y - 4, 
			rct->x2 + 4, rct->y2 + 4);			
		}
	}
}

void update_srn()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	
	draw_btn();
	
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
	window = SDL_CreateWindow("sand",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,window_width,window_height,SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	SDL_Event event;
	
	for (;;){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:
					goto jmp;
			}			
		}
		
		update_st();
		update_srn();
	}
	jmp:
	
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	
	return 0;
}

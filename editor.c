#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
	int length; //malloc length
	//int vlength; //virtual length
	char *data;
	bool dirty;
	SDL_Surface *render;
} 
BUFFER_LINE;

typedef struct
{
	int cursor_row;
	int cursor_column;
	int nlines;
	char path[FILENAME_MAX]; //max chars for a windows path, maybe
	BUFFER_LINE **rows;
} 
BUFFER;

#define SIZEOF_BUFFER sizeof(BUFFER)
#define SIZEOF_BUFFER_PTR sizeof(BUFFER*)
#define SIZEOF_BUFFER_LINE sizeof(BUFFER_LINE)

#define NEWLINE '\n'
#define TAB '\t'
#define SPACE ' '

SDL_Window *WINDOW;
SDL_Renderer *RENDERER;

TTF_Font *FONT;

BUFFER *CURRENT_BUFFER;

BUFFER_LINE *add_line(BUFFER *in, char *source, int length)
{
	int plines = in->nlines;

	in->nlines++;
	in->rows = realloc(in->rows, SIZEOF_BUFFER_PTR * in->nlines);
	BUFFER_LINE *line = malloc(SIZEOF_BUFFER_LINE);
	in->rows[plines] = line;

	memset(line, 0, SIZEOF_BUFFER_LINE);

	//copy our data
	if (length > 0)
	{
		line->length = length;
		line->data = malloc(length);
		
		memmove(line->data, source, length);
	}

	return line;
}

BUFFER *load_file(char *source)
{
	FILE *file = fopen(source, "r");
	if (file == NULL)
	{
		return NULL;
	}
	//check buf isnt NULL
	BUFFER *buf = malloc(SIZEOF_BUFFER);
	size_t slen = strlen(source);

	memset(buf, 0, SIZEOF_BUFFER);

	if (slen > FILENAME_MAX)
	{
		slen = FILENAME_MAX;
	}

	memmove(buf->path, source, slen);

	long int pos = 0;
	long int length = 0;
	int byte;

	for (;;)
	{
		byte = fgetc(file);
		if (byte == NEWLINE)
		{
			//remember where we stopped
			//so we can start there again

			BUFFER_LINE *line = add_line(buf, NULL, 0);

			//zero length not accounted for

			long int eol = ftell(file);

			line->data = malloc(length);	
			line->length = length;

			fseek(file, pos, SEEK_SET);
			fread(line->data, 1, length, file);

			length = 0;
			pos = eol;

			fseek(file, eol, SEEK_SET);
			continue;
		}

		if (byte == EOF)
		{
			BUFFER_LINE *line = add_line(buf, NULL, 0);
			line->data = malloc(length);	
			line->length = length;

			fseek(file, pos, SEEK_SET);
			fread(line->data, 1, length, file);
			break;
		}
		
		length++;
	}

	if (buf->nlines == 0)
	{
		add_line(buf, NULL, 0);
	}
	
	fclose(file);
	
	return buf;
}

void export_buffer(BUFFER *buf)
{
	if (buf->nlines == 0)
	{
		return;
	}

	FILE *file = fopen(buf->path, "w");

	remove(buf->path);

	if (file == NULL)
	{
		return;
	}

	for (int i = 0; i < buf->nlines; i++)
	{
		BUFFER_LINE *line = buf->rows[i];

		fwrite(line->data, 1, line->length, file);
	
		if (i < buf->nlines - 1)
		{
			fputc(NEWLINE, file);
		}
	}

	fclose(file);
}

int text_length(char *text, int len)
{
	int text_len;
	if (len == 1)
	{
		TTF_GlyphMetrics(FONT, text[0], NULL, NULL, NULL, NULL, &text_len);
		return text_len;
	}
	
	Uint16 ch;
	text_len = 0;
	for (int i = 0; i < len; i ++)
	{
		ch = text[i];
		int ch_adv;
		if (ch == TAB)
		{
			TTF_GlyphMetrics(FONT, SPACE, NULL, NULL, NULL, NULL, &ch_adv);
		} else
		{
			TTF_GlyphMetrics(FONT, ch, NULL, NULL, NULL, NULL, &ch_adv);
		}
		text_len += ch_adv;
	}
	return text_len;
}

void draw_text(char *text, int tsize, int x, int y)
{
	
}

void display(BUFFER *buf)
{
	//todo
	
}

int main(int argc, char *argv[]) 
{
	//BUFFER *buf = load_file(argv[1]);
	BUFFER *buf = load_file("text.txt");

	if (buf == NULL)
	{
		perror("could not load file");

		return 0;
	}

	TTF_Init();

	FONT = TTF_OpenFont("C:/Windows/Fonts/Arial.ttf", 16);

	if (FONT == NULL)
	{
		perror("could not load font");
		TTF_Quit();

		return 0;
	}

	SDL_Init(SDL_INIT_VIDEO);

	WINDOW = SDL_CreateWindow("",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,512,512,0);
	RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;

	for (;;)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:	
					goto quit;
				
			}
		}
		
	}

	quit:

	SDL_DestroyWindow(WINDOW);
	SDL_DestroyRenderer(RENDERER);
	SDL_Quit();
	TTF_Quit();

	return 0;
}

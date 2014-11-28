#include <math.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>

#define member_sizeof(s, m) sizeof(((s*) 0)->m)

int x_pos = 0;//TODO: make non-global
int y_pos = 0;

bool lmb = false;
bool prev_lmb = false;

bool enter = false;
bool prev_enter = false;

SDL_Window * window;
SDL_Renderer * renderer;

#define default_background 25, 25, 25, 255
SDL_Color background = { default_background };

TTF_Font * font;
TTF_Font * button_font;

struct textbox{
	int offset;
	char string[100];
};

textbox * selected_textbox = 0;
int cursor;
int select_cursor;

char message[150];
uint32_t message_time = 0;

inline int min(int a, int max){
	if (a < max){
		return a;
	}
	return max;
}

inline int max(int a, int min){
	if (a > min){
		return a;
	}
	return min;
}

//greatest common devisor using Euclid's algorithm
int gcd(int a, int b){
	while (b != 0){
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}

void SDL_RenderFillRoundyRect(SDL_Renderer * rndrr, SDL_Rect * box, int r){//copied SDLs format for easy switching //memoize if optimization is needed
	SDL_RenderFillRect(rndrr, box);
#if 1//Xiaolin Wu
	float y_c;
	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	for (float x_c = 0; x_c <= 1+(float)r/sqrt(2.0); x_c+=1){
		SDL_SetRenderDrawColor(rndrr, background.r, background.g, background.b, background.a);
		y_c = sqrt(r*r-x_c*x_c);
		int y_i = ceil(y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + r - y_c, box->x + r - x_c, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + r - x_c, box->x + r - y_c, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + box->h - r + y_c, box->x + r - x_c, box->y + box->h - r + y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + box->h - r + x_c - 1, box->x + r - y_c, box->y + box->h - r + x_c - 1);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_c, box->x + box->w, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c, box->y + r - x_c, box->x + box->w, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_c, box->x + box->w, box->y + box->h - r + y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c, box->y + box->h - r + x_c - 1, box->x + box->w, box->y + box->h - r + x_c - 1);
		
		//SDL_SetRenderDrawColor(rndrr, background.r*(ceil(y_c) - y_c) + 180 * (y_c - floor(y_c)), background.g *(ceil(y_c) - y_c) + 180 * (y_c - floor(y_c)), background.b*(ceil(y_c) - y_c) + 180 * (y_c - floor(y_c)), background.a);
		float alpha = y_i - y_c;
		SDL_SetRenderDrawColor(rndrr, c_r, c_g, c_b, (1.0-alpha)*c_a);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + r - y_i);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_i, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + box->h - r + y_i - 1);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_i, box->y + box->h - r + x_c - 1);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_i);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_i - 1, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_i - 1);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_i - 1, box->y + box->h - r + x_c - 1);
	}
#else//Bressenham
	int x_c = r;
	int err = 1 - r;
	//unsigned char c_r, c_g, c_b, c_a;
	//SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	SDL_SetRenderDrawColor(rndrr, background.r, background.g, background.b, background.a);
	for (int y_c = 0; y_c <= x_c; y_c++){
		SDL_RenderDrawLine(rndrr, box->x - 1, box->y + r - y_c, box->x + r - x_c, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x - 1, box->y + r - x_c, box->x + r - y_c, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x - 1, box->y + box->h - r + y_c - 1, box->x + r - x_c, box->y + box->h - r + y_c - 1);
		SDL_RenderDrawLine(rndrr, box->x - 1, box->y + box->h - r + x_c - 1, box->x + r - y_c, box->y + box->h - r + x_c - 1);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_c, box->x + box->w, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c - 1, box->y + r - x_c, box->x + box->w, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_c - 1, box->x + box->w, box->y + box->h - r + y_c - 1);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c - 1, box->y + box->h - r + x_c - 1, box->x + box->w, box->y + box->h - r + x_c - 1);
		/*fake AA that doesn't work that well
		float error = abs(sqrtf(r*r) - sqrtf(x_c*x_c + y_c*y_c));//the distance form the real circle
		SDL_SetRenderDrawColor(rndrr, (background.r*error + c_r) / (1 + error), (background.g*error + c_g) / (1 + error), (background.b*error + c_b) / (1 + error), background.a);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + r - y_c);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_c, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + box->h - r + y_c - 1);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_c, box->y + box->h - r + x_c - 1);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_c);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_c - 1, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_c - 1);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_c - 1, box->y + box->h - r + x_c - 1);
		*/
		if (err < 0){
			err += 2 * y_c + 1;
		}
		else{
			err += 2 * (y_c - x_c + 1);
			x_c--;
		}
	}
#endif
}
void SDL_RenderFillLeftRoundyRect(SDL_Renderer * rndrr, SDL_Rect * box, int r){//copied SDLs format for easy switching //memoize if optimization is needed
	SDL_RenderFillRect(rndrr, box);
//Xiaolin Wu
	float y_c;
	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	for (float x_c = 0; x_c <= 1 + (float)r / sqrt(2.0); x_c += 1){
		SDL_SetRenderDrawColor(rndrr, background.r, background.g, background.b, background.a);
		y_c = sqrt(r*r - x_c*x_c);
		int y_i = ceil(y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + r - y_c, box->x + r - x_c, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + r - x_c, box->x + r - y_c, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + box->h - r + y_c, box->x + r - x_c, box->y + box->h - r + y_c);
		SDL_RenderDrawLine(rndrr, box->x-1, box->y + box->h - r + x_c - 1, box->x + r - y_c, box->y + box->h - r + x_c - 1);

		float alpha = y_i - y_c;
		SDL_SetRenderDrawColor(rndrr, c_r, c_g, c_b, (1.0 - alpha) * c_a);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + r - y_i);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_i, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + r - x_c, box->y + box->h - r + y_i - 1);
		SDL_RenderDrawPoint(rndrr, box->x + r - y_i, box->y + box->h - r + x_c - 1);
	}
}
void SDL_RenderFillRightRoundyRect(SDL_Renderer * rndrr, SDL_Rect * box, int r){//copied SDLs format for easy switching //memoize if optimization is needed
	SDL_RenderFillRect(rndrr, box);
	//Xiaolin Wu
	float y_c;
	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	for (float x_c = 0; x_c <= 1 + (float)r / sqrt(2.0); x_c += 1){
		SDL_SetRenderDrawColor(rndrr, background.r, background.g, background.b, background.a);
		y_c = sqrt(r*r - x_c*x_c);
		int y_i = ceil(y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_c, box->x + box->w, box->y + r - y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c, box->y + r - x_c, box->x + box->w, box->y + r - x_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_c, box->x + box->w, box->y + box->h - r + y_c);
		SDL_RenderDrawLine(rndrr, box->x + box->w - r + y_c, box->y + box->h - r + x_c - 1, box->x + box->w, box->y + box->h - r + x_c - 1);

		float alpha = y_i - y_c;
		SDL_SetRenderDrawColor(rndrr, c_r, c_g, c_b, (1.0 - alpha) * c_a);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + r - y_i);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_i - 1, box->y + r - x_c);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + x_c - 1, box->y + box->h - r + y_i - 1);
		SDL_RenderDrawPoint(rndrr, box->x + box->w - r + y_i - 1, box->y + box->h - r + x_c - 1);
	}
}

void clearTextboxSelection(){
	SDL_memcpy(
		selected_textbox->string + min(cursor, select_cursor), 
		selected_textbox->string + max(cursor, select_cursor),
		SDL_strlen(selected_textbox->string) - max(cursor, select_cursor) + 1//possibly clamp to 0, only if the cursor can ever get out of the text length
	);
	cursor = min(cursor, select_cursor);
}

void copyTextboxSelection(){
	char clip[100];
	SDL_memcpy(
		clip,
		selected_textbox->string + min(cursor, select_cursor),
		max(cursor, select_cursor) - min(cursor, select_cursor)
		);
	clip[max(cursor, select_cursor) - min(cursor, select_cursor)] = 0;
	SDL_SetClipboardText(clip);
}

void typeIntoSelectedTextbox(char * string){
	if (selected_textbox == 0){
		return;
	}
	clearTextboxSelection();
	int max_cpy = member_sizeof(textbox, string) - cursor - SDL_strlen(string)-1;//the distance form the cursor + new text to the end of the box
	if (SDL_strlen(selected_textbox->string) - cursor > 0 && max_cpy > 0){//insert in the middle of the text
		SDL_memcpy(
			selected_textbox->string + cursor + SDL_strlen(string),
			selected_textbox->string + cursor,
			min(SDL_strlen(selected_textbox->string) + 1 - cursor, max_cpy)
		);
	}
	if (member_sizeof(textbox, string) - 1 - cursor > 0){//if not at end
		if (cursor == SDL_strlen(selected_textbox->string)){
			selected_textbox->string[SDL_strlen(selected_textbox->string)+1] = 0;
		}
		SDL_memcpy(
			selected_textbox->string + cursor,//dst
			string,//src
			min(SDL_strlen(string), member_sizeof(textbox, string) - 1 - cursor)//len
		);
	}
	cursor += SDL_strlen(string);
	cursor = min(cursor, member_sizeof(textbox, string) - 1);
	select_cursor = cursor;
}

//width must be greater than height
bool doTextBox(textbox * t, int width, int height){
	int char_width;
	TTF_GlyphMetrics(font, '_', 0, 0, 0, 0, &char_width);

	int mx, my;
	SDL_GetMouseState(&mx, &my);
	if (lmb){
		if (selected_textbox == t){
			if ((mx >= x_pos && mx < x_pos + width) || SDL_GetTicks()%10==0){
				cursor = max(min((max(min(mx, x_pos + width + char_width), x_pos - char_width) - x_pos + t->offset) / char_width, SDL_strlen(t->string)), 0);
			}
		}
		if (!prev_lmb && mx >= x_pos && my >= y_pos && mx < x_pos+width && my < y_pos+height){
			cursor = max(min((mx - x_pos + t->offset) / char_width, SDL_strlen(t->string)), 0);
			selected_textbox = t;
			select_cursor = cursor;
		}
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_Color color = { 0, 0, 0 };
	int y_padd = (height - TTF_FontHeight(font)) / 2;
	int x_padd = y_padd;// char_width / 2;
	SDL_Rect a = { x_pos, y_pos, width, height};
	SDL_RenderFillLeftRoundyRect(renderer, &a, 4);
	a.y += y_padd;
	a.h = TTF_FontHeight(font);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	if (selected_textbox == t){
		int off_to_cursor_width = char_width*cursor - t->offset;
		int x = off_to_cursor_width + x_pos + x_padd;
		if (off_to_cursor_width > width - 2 * x_padd){
			t->offset += (off_to_cursor_width - (width - 2 * x_padd));
			x = width - x_padd;
		}
		if (off_to_cursor_width < 0){
			t->offset = char_width*cursor;
			x = x_padd;
		}
		if ((SDL_GetTicks() / 500) & 1){
			SDL_RenderDrawLine(renderer, x, y_pos + y_padd, x, y_pos + height - y_padd);
		}
		if (cursor != select_cursor){
			int select_x = max(min(char_width*select_cursor - t->offset + x_pos + x_padd, x_pos - x_padd + width), x_pos + x_padd);
			SDL_SetRenderDrawColor(renderer, 50, 100, 255, 125);
			SDL_Rect select_box = { min(x, select_x), y_pos + y_padd / 2, max(x, select_x) - min(x, select_x), height + 1 - y_padd };
			SDL_RenderFillRect(renderer, &select_box);
		}
	}

	SDL_Surface * surface = TTF_RenderText_Solid(font, t->string, color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_Rect b;
	SDL_GetClipRect(surface, &b);
	b.x += t->offset;
	b.w -= b.x;
	if (b.w > width-2*x_padd){
		b.w = width-2*x_padd;
	}
	a.x += x_padd;
	a.w = b.w;

	SDL_RenderCopy(renderer, texture, &b, &a);
	
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	return t == selected_textbox && enter && !prev_enter;
}

bool doButton(char * name, int width, int height){
	SDL_Rect box = { x_pos, y_pos, width, height };
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	bool over = mx >= x_pos && my >= y_pos && mx < x_pos + width && my < y_pos + height;
	bool pressed = lmb && over;
	if (pressed){
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	}
	else if (over){
		SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
	}
	else{
		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	}
	SDL_RenderFillRightRoundyRect(renderer, &box, 4);

	SDL_Color color = { 0, 0, 0 };
	SDL_Surface * surface = TTF_RenderText_Solid(button_font, name, color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_Rect b;
	SDL_GetClipRect(surface, &b);
	b.x = x_pos + (width - b.w) / 2;
	b.y = y_pos + (height - b.h) / 2;

	SDL_RenderCopy(renderer, texture, 0, &b);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	return over && !lmb && prev_lmb;
}

void doSelectionButton(char * names, int n, int * selected, int width, int height){
	int mx, my;
	width /= n;
	SDL_GetMouseState(&mx, &my);
	for (int i = 0; i < n; i++, names += SDL_strlen(names)+1){
		bool over = mx >= x_pos+width*i && my >= y_pos && mx < x_pos + width*(i+1) && my < y_pos + height;
		SDL_Rect box = { x_pos + width*i, y_pos, width, height };
		bool pressed = lmb && over;
		if (over && !lmb && prev_lmb){
			*selected = i;
		}
		if (pressed){
			SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
		}
		else if (over || *selected == i){
			SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
		}
		else{
			SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
		}
		if (i == 0){
			SDL_RenderFillLeftRoundyRect(renderer, &box, 4);
		}
		else if (i == n-1){
			SDL_RenderFillRightRoundyRect(renderer, &box, 4);
		}
		else{
			SDL_RenderFillRect(renderer, &box);
		}

		if (i > 0){
			SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
			SDL_RenderDrawLine(renderer, x_pos + width*i, y_pos, x_pos + width*i, y_pos + height);
		}

		SDL_Color color = { 0, 0, 0 };
		SDL_Surface * surface = TTF_RenderText_Solid(button_font, names, color);
		SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect b;
		SDL_GetClipRect(surface, &b);
		b.x = x_pos+width*i+(width - b.w) / 2;
		b.y = y_pos+(height - b.h) / 2;

		SDL_RenderCopy(renderer, texture, 0, &b);

		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
	}
}

int * selected_slider = 0;
int * prev_selected_slider = 0;
bool doSlider(char * name, int * value, int max_value, int width, int height){
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	bool over = mx >= x_pos && my >= y_pos && mx < x_pos + width && my < y_pos + height;
	if (selected_slider == value || prev_selected_slider == value){
		*value = max(0, min(max_value, round(float(mx-x_pos-5-(width/max_value/2))*max_value / (width - width / max_value - 10))));
	}
	if (over && lmb && !prev_lmb){
		selected_slider = value;
	}

	SDL_Rect box = { x_pos, y_pos, width, height };

	if (selected_slider == value){
		SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
	}
	else if (over){
		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	}
	else{
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	}
	SDL_RenderFillRoundyRect(renderer, &box, 4);

	SDL_Color color = { 0, 0, 0 };
	SDL_Surface * surface = TTF_RenderText_Solid(button_font, name, color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_GetClipRect(surface, &box);
	box.x = x_pos+(width - box.w) / 2;
	box.y = y_pos+(height - box.h) / 2;

	SDL_RenderCopy(renderer, texture, 0, &box);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	SDL_SetRenderDrawColor(renderer, 140, 21, 21, 255);
	int y_padd = min(height / 20, 5);
	box = { x_pos + 5 + (width - (width - 10)/max_value - 10)*(*value) / max_value, y_pos + y_padd, (width - 10) / max_value, height - 2 * y_padd };
	SDL_RenderFillRect(renderer, &box);

	if (selected_slider == value){
		SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
		SDL_Color color = { 255, 255, 255 };
		char val[100];
		int xnum = 2 * (*value) - 15;
		int xden = 2 * max_value;
		int gcdx = abs(gcd(xnum, xden));
		xnum /= gcdx;
		xden /= gcdx;
		//format all the special cases {except denominator = -1 since it's always positive}
		if (xnum == 0){ sprintf_s(val, "0"); }
		else if (xnum*xden == 1){ sprintf_s(val, " \xCF\x80 "); }
		else if (xnum*xden == -1){ sprintf_s(val, " -\xCF\x80 "); }
		else if (xnum == 1){ sprintf_s(val, " \xCF\x80/%i ", xden); }
		else if (xnum == -1){ sprintf_s(val, " -\xCF\x80/%i ", xden); }
		else if (xden == 1){ sprintf_s(val, " %i\xCF\x80 ", xnum); }
		else { sprintf_s(val, " %i\xCF\x80/%i ", xnum, xden); }//general case

		SDL_Surface * surface = TTF_RenderUTF8_Solid(button_font, val, color);
		SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect box;
		SDL_GetClipRect(surface, &box);
		box.x = x_pos+width/2-box.w/2;
		box.y = y_pos+height-TTF_FontHeight(button_font)-height/10;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
		SDL_RenderFillRect(renderer, &box);
		
		SDL_RenderCopy(renderer, texture, 0, &box);
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
	}
	return (prev_selected_slider == value && selected_slider != value);
}

void SDL_RenderFillDownTriangle(SDL_Renderer * renderer, SDL_Rect * box){
	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	if (box->w >= 2 * box->h){
		for (int x = 0; x <= box->w; x++){
			float y = box->y + box->h - (abs((float)2 * x - box->w)*box->h) / box->w;
			int y_i = ceil(y);
			SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, c_a);
			SDL_RenderDrawLine(renderer, x + box->x, box->y, x + box->x, y_i);
			SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, background.a);
			SDL_RenderDrawPoint(renderer, x + box->x, y_i);
			SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, (1 - (y_i - y))*c_a);
			SDL_RenderDrawPoint(renderer, x + box->x, y_i);
		}
	}
}
void SDL_RenderFillUpTriangle(SDL_Renderer * renderer, SDL_Rect * box){
	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);
	if (box->w >= 2 * box->h){
		for (int x = 0; x <= box->w; x++){
			float y = box->y + (abs((float)2 * x - box->w)*box->h) / box->w;
			int y_i = ceil(y);
			SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, c_a);
			SDL_RenderDrawLine(renderer, x + box->x, box->y+box->h, x + box->x, y_i);
			SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, background.a);
			SDL_RenderDrawPoint(renderer, x + box->x, y_i);
			SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, (1 - (y_i - y))*c_a);
			SDL_RenderDrawPoint(renderer, x + box->x, y_i);
		}
	}
}

bool doToggleButton(bool * toggle, int width, int height){
	SDL_Rect box = { x_pos, y_pos, width, height };
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	bool over = mx >= x_pos && my >= y_pos && mx < x_pos + width && my < y_pos + height;
	bool pressed = lmb && over;
	if (over && !lmb && prev_lmb){
		*toggle = !(*toggle);
	}
	if (pressed){
		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	}
	else if (over){
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	else{
		SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
	}
	if(*toggle){
		SDL_RenderFillDownTriangle(renderer, &box);
	}
	else{
		SDL_RenderFillUpTriangle(renderer, &box);
	}

	return over && !lmb && prev_lmb;
}

struct grid{
	int x;
	int y;
};

bool doGrid(grid * g, int x_cells, int y_cells, int width, int height){
	bool out = false;
	x_cells--;
	y_cells--;

	unsigned char c_r, c_g, c_b, c_a;
	SDL_GetRenderDrawColor(renderer, &c_r, &c_g, &c_b, &c_a);

	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_Rect back = { x_pos, y_pos, width, height };
	SDL_RenderFillRoundyRect(renderer, &back, 4);
	SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, c_a);

	int mx, my;
	SDL_GetMouseState(&mx, &my);
	bool over = mx >= x_pos && my >= y_pos && mx < x_pos + width && my < y_pos + height;

	int marker_width = min(width / x_cells, 10);
	int marker_height = min(height / y_cells, 10);

	width *= (float)x_cells/(1+x_cells);
	height *= (float)y_cells/(1+y_cells);
	x_pos += (width*0.5)/x_cells;
	y_pos += (height*0.5)/y_cells;

	int over_x = max(min(roundf(float(mx - x_pos)*x_cells / width), x_cells), 0);
	int over_y = max(min(roundf(float(y_pos+height-my)*y_cells / height), y_cells), 0);

	for (int i = 0; i <= x_cells; i++){
		int x = x_pos + width*((float)i/x_cells);
		SDL_RenderDrawLine(renderer, x, y_pos, x, y_pos + height);
	}
	for (int i = 0; i <= y_cells; i++){
		int y = y_pos + height*((float)i / y_cells);
		SDL_RenderDrawLine(renderer, x_pos, y, x_pos + width, y);
	}
	if (over){
		SDL_Color color = { 255, 255, 255 };
		char coord[100];
		int xnum = 2 * over_x - 15;
		int xden = 2 * x_cells;
		int ynum = 2 * over_y - 15;
		int yden = 2 * y_cells;
		int gcdx = abs(gcd(xnum, xden));
		xnum /= gcdx;
		xden /= gcdx;
		int gcdy = abs(gcd(ynum, yden));
		ynum /= gcdy;
		yden /= gcdy;
		//format all the special cases {except denominator = -1 since it's always positive}
		char x_coord[50];
		if (xnum == 0){sprintf_s(x_coord, "0");}
		else if (xnum*xden == 1){sprintf_s(x_coord, "\xCF\x80");}
		else if (xnum*xden == -1){sprintf_s(x_coord, "-\xCF\x80");}
		else if (xnum == 1){sprintf_s(x_coord, "\xCF\x80/%i", xden);}
		else if (xnum == -1){sprintf_s(x_coord, "-\xCF\x80/%i", xden);}
		else if (xden == 1){sprintf_s(x_coord, "%i\xCF\x80", xnum);}
		else {sprintf_s(x_coord, "%i\xCF\x80/%i", xnum, xden);}//general case
		char y_coord[50];
		if (ynum == 0){sprintf_s(y_coord, "0");}
		else if (ynum*yden == 1){sprintf_s(y_coord, "\xCF\x80");}
		else if (ynum*yden == -1){sprintf_s(y_coord, "-\xCF\x80");}
		else if (ynum == 1){sprintf_s(y_coord, "\xCF\x80/%i", yden);}
		else if (ynum == -1){sprintf_s(y_coord, "-\xCF\x80/%i", yden);}
		else if (yden == 1){sprintf_s(y_coord, "%i\xCF\x80", ynum);}
		else {sprintf_s(y_coord, "%i\xCF\x80/%i", ynum, yden);}//general case

		sprintf_s(coord, " (%s, %s) ", x_coord, y_coord);
		SDL_Surface * surface = TTF_RenderUTF8_Solid(button_font, coord, color);
		SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect box;
		SDL_GetClipRect(surface, &box);
		box.x = x_pos + over_x*width / x_cells-10;
		box.y = y_pos + height - (over_y+0.5)*height / y_cells-2-TTF_FontHeight(button_font)/2;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
		SDL_RenderFillRect(renderer, &box);

		SDL_RenderCopy(renderer, texture, 0, &box);
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);

		if (lmb){
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		}
		else{
			SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
		}
		SDL_Rect active = { x_pos + over_x*width / x_cells - marker_width/2, y_pos + height - over_y*height / y_cells - marker_height/2, marker_width, marker_height };
		SDL_RenderFillRect(renderer, &active);
		if (prev_lmb && !lmb){
			g->x = over_x;
			g->y = over_y;
			out = true;
		}
	}
	SDL_SetRenderDrawColor(renderer, c_r, c_g, c_b, c_a);
	SDL_Rect current = { x_pos + g->x*width / x_cells - marker_width/2, y_pos + height - g->y*height / y_cells - marker_height/2, marker_width, marker_height };
	SDL_RenderFillRect(renderer, &current);
	return out;
}

struct netStruct{
	char host[100];
	IPaddress * ipa;
	TCPsocket * tcpsock;
	unsigned char data;
};

static int connect(void * nsv){
	netStruct * ns = (netStruct *)nsv;
	if (SDLNet_ResolveHost(ns->ipa, ns->host, 5204) == -1){
		sprintf_s(message, "Error connecting to %s: %s", ns->host, SDLNet_GetError());
		return 0;
	}
	else{
		(*ns->tcpsock) = SDLNet_TCP_Open(ns->ipa);
		if (!(*ns->tcpsock)){
			sprintf_s(message, "Error connecting to %s: %s", ns->host, SDLNet_GetError());
			return 0;
		}
		sprintf_s(message, "Connected to %s", ns->host);
		message_time = SDL_GetTicks();
	}
	while (*ns->tcpsock){
		unsigned char data[100];
		int len = SDLNet_TCP_Recv(*ns->tcpsock, data, 100);
		if (len > 0){
			ns->data = data[len-1];
		}
		else{
			sprintf_s(message, "Connection Lost");
			*ns->tcpsock = 0;
		}
	}
	return 0;
}

int main(int argc, char* argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDLNet_Init();

	window = SDL_CreateWindow(
		"Parker's Neck",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
	);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	{
		SDL_Surface * icon = IMG_Load("Shrubbery32.ico");
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}

	if (TTF_Init() == -1){
		return 1;
	}
	font = TTF_OpenFont("LiberationMono-Regular.ttf", 11);
	if (!font){//should probably start using asserts
		return 0;
	}

	button_font = TTF_OpenFont("arial.ttf", 11);
	if (!button_font){//should probably start using asserts
		return 0;
	}

	textbox ip_box = { 0, "192.168.0.1" };
	grid g = { 0, 0 };
	SDL_Event event;
	bool expanded = 1;
	int mode = 0;
	IPaddress ipa;
	TCPsocket tcpsock = {};
	SDL_Thread * netthread;
	netStruct * ns = new netStruct;
	while (1){
		//poll all events
		while (SDL_PollEvent(&event)){
			switch (event.type){
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT){
					selected_textbox = 0;
					selected_slider = 0;
					lmb = true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT){
					selected_slider = 0;
					lmb = false;
				}
				break;
			case SDL_TEXTINPUT:
				typeIntoSelectedTextbox(event.text.text);
				select_cursor = cursor;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode){
				case SDL_SCANCODE_RETURN:
					enter = false;
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode){
				case SDL_SCANCODE_RETURN:
					enter = true;
					break;
				case SDL_SCANCODE_LEFT:
					if (cursor > 0){
						cursor--;
					}
					if (!(event.key.keysym.mod & KMOD_SHIFT)){
						select_cursor = cursor;
					}
					break;
				case SDL_SCANCODE_RIGHT:
					if (cursor < SDL_strlen(selected_textbox->string)){
						cursor++;
					}
					if (!(event.key.keysym.mod & KMOD_SHIFT)){
						select_cursor = cursor;
					}
					break;
				case SDL_SCANCODE_BACKSPACE:
					if (cursor >= 0 && select_cursor >= 0 && selected_textbox){
						if (select_cursor != cursor){
							clearTextboxSelection();
						}
						else if (cursor > 0 && selected_textbox){
							SDL_memcpy(selected_textbox->string + cursor - 1, selected_textbox->string + cursor, SDL_strlen(selected_textbox->string) - cursor + 1);
							cursor--;
						}
						select_cursor = cursor;
					}
					break;
				case SDL_SCANCODE_DELETE:
					if (selected_textbox){
						SDL_memcpy(selected_textbox->string + cursor, selected_textbox->string + cursor + 1, SDL_strlen(selected_textbox->string) - cursor + 1);
					}
					break;
				case SDL_SCANCODE_V:
					if ((event.key.keysym.mod & KMOD_CTRL) && SDL_HasClipboardText()){
						typeIntoSelectedTextbox(SDL_GetClipboardText());
					}
					break;
				case SDL_SCANCODE_X:
					if ((event.key.keysym.mod & KMOD_CTRL) && SDL_HasClipboardText()){
						copyTextboxSelection();
						clearTextboxSelection();
					}
					break;
				case SDL_SCANCODE_C:
					if ((event.key.keysym.mod & KMOD_CTRL)){
						copyTextboxSelection();
					}
					break;
				}
				break;
			case SDL_QUIT:
				return 0;
				break;//this break isn't necessary, since the return will exit the program
			}
		}
		int window_width, window_height;
		SDL_GetWindowSize(window, &window_width, &window_height);

		background = { default_background };
		SDL_SetRenderDrawColor(renderer, default_background);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		if (expanded){
			SDL_SetRenderDrawColor(renderer, 140, 21, 21, 255);
			background = { 140, 21, 21, 255 };
			SDL_Rect a = { 0, window_height - 40, window_width, 40 };
			SDL_RenderFillRect(renderer, &a);

			x_pos = 10;
			y_pos = window_height - 30;
			bool ip_entered = doTextBox(&ip_box, window_width-222, 20);
			x_pos += window_width - 222;
			ip_entered = doButton("Connect", 64, 20) || ip_entered;//TODO: add vertical scrolling if it goes outside the window overflowing
			if (ip_entered && ip_box.string[0]){
				sprintf_s(message, "Connecting to %s ...", ip_box.string);
				SDL_strlcpy(ns->host, ip_box.string, 100);
				ns->ipa = &ipa;
				ns->tcpsock = &tcpsock;
				netthread = SDL_CreateThread(connect, "NetThread", (void *)ns);
				if (netthread){
					//SDL_DetachThread(netthread);
				}
			}

			x_pos += 64 + 10;
			doSelectionButton("Grid View\0Slider View", 2, &mode, 128, 20);
		}
		x_pos = window_width / 2 - 5;
		y_pos = window_height - 8;
		doToggleButton(&expanded, 10, 5);
		if (expanded){
			window_height -= 40;
			background = { default_background };
		}

		if (tcpsock){
			g.x = ns->data >> 4;
			g.y = ns->data & 0x0F;
		}
		else{
			message_time = 0;
		}

		switch (mode){
		case 0:
			SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
			x_pos = max((window_width - min(window_width, window_height)) / 2, 0) + 10;
			y_pos = max((window_height - min(window_width, window_height)) / 2, 0) + 10;
			if (doGrid(&g, 16, 16, min(window_width, window_height) - 20, min(window_width, window_height) - 20) && tcpsock){
				ns->data = (g.x << 4) | (g.y & 0x0F);
				if (!SDLNet_TCP_Send(tcpsock, &ns->data, 1)){
					sprintf_s(message, "Connection Lost");
					tcpsock = 0;
				}
			}
			break;
		case 1:
			x_pos = 10;
			y_pos = 10;
			bool send = doSlider("Longitude", &g.x, 15, window_width - 20, (window_height - 30)/2);
			y_pos += (window_height - 30) / 2+10;
			send = doSlider("Latitude", &g.y, 15, window_width - 20, (window_height-30)/2) || send;
			if (send && tcpsock){
				ns->data = (g.x << 4) | (g.y & 0x0F);
				if (!SDLNet_TCP_Send(tcpsock, &ns->data, 1)){
					sprintf_s(message, "Connection Lost");
					tcpsock = 0;
				}
			}
			break;
		}

		if (message[0] && (SDL_GetTicks() - message_time <= 10000 || message_time == 0)){
			int mx, my;
			SDL_GetMouseState(&mx, &my);

			SDL_Color color = { 255, 255, 255 };
			SDL_Surface * surface = TTF_RenderText_Solid(font, message, color);
			SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

			SDL_Rect box;
			SDL_GetClipRect(surface, &box);
			box.x = window_width - box.w - 10;

			SDL_Rect b;
			b.w = box.w + 20;
			b.h = 20;
			b.x = window_width - b.w;
			b.y = window_height - b.h;

			box.y = window_height - (b.h + box.h) / 2;

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
			SDL_RenderFillRect(renderer, &b);

			SDL_RenderCopy(renderer, texture, 0, &box);
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(surface);
		}

		SDL_RenderPresent(renderer);
		prev_lmb = lmb;
		prev_enter = enter;
		prev_selected_slider = selected_slider;
		SDL_Delay(16-(SDL_GetTicks()%8));//125 fps
	}

	//don't know why these are needed so I got rid of them
	//SDL_DestroyWindow(window);//for multiple windows?
	//SDL_Quit();//the programs ending what do you need to do? mabey if you want to do something after your done with all the sdl stuff? but why would you want to do that?
	return 0;
}
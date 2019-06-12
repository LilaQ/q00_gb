void initPPU();
SDL_Window* getWindow();
void drawLine(unsigned char memory[]);
void drawBGTileset(unsigned char memory[], SDL_Renderer *tRenderer, SDL_Window* tWindow);
void drawBGMap(unsigned char memory[], SDL_Renderer* tRenderer, SDL_Window* tWindow);
void stopPPU();
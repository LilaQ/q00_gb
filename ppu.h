void initPPU();
SDL_Window* getWindow();
void drawLine();
void drawBGTileset(SDL_Renderer *tRenderer, SDL_Window* tWindow);
void drawBGMap(SDL_Renderer* tRenderer, SDL_Window* tWindow);
void drawWindowMap(SDL_Renderer* tRenderer, SDL_Window* tWindow);
void drawSpriteMap(SDL_Renderer* tRenderer, SDL_Window* tWindow);
void stopPPU();
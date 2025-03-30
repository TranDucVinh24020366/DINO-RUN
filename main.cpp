#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <fstream>
using namespace std;

struct Position {
    int x, y;
};

struct Fireball {
    Position pos;
    float velocityX, velocityY;
    bool active;
    bool scored;
};

struct Star {
    Position pos;
    float velocityY;
    bool active;
};

// Hàm đọc highscore từ file
int loadHighscore() {
    ifstream file("highscore.txt");
    int highscore = 0;
    if (file.is_open()) {
        file >> highscore;
        file.close();
    }
    return highscore;
}

// Hàm lưu highscore vào file
void saveHighscore(int highscore) {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highscore;
        file.close();
    } else {
        cerr << "Khong the mo file de luu highscore!" << endl;
    }
}

int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL khong the khoi tao! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG))) {
        cerr << "SDL_image khong the khoi tao! IMG_Error: " << IMG_GetError() << endl;
        return 1;
    }
    if (TTF_Init() < 0) { // Khởi tạo SDL_ttf
        cerr << "SDL_ttf khong the khoi tao! TTF_Error: " << TTF_GetError() << endl;
        return 1;
    }

    // Tạo cửa sổ và renderer
    SDL_Window* window = SDL_CreateWindow("DINO RUN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 576, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Khong the tao cua so! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer khong the tao ra! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    // Tải font
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Đường dẫn đến file font (cần có file arial.ttf)
    if (!font) {
        cerr << "Khong the tai font! TTF_Error: " << TTF_GetError() << endl;
        return 1;
    }

    // Tải ảnh
    SDL_Texture* back0_texture = IMG_LoadTexture(renderer, "images/back0.png");
    if (!back0_texture) {
        cerr << "Khong the tai back0.png! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Texture* back1_texture = IMG_LoadTexture(renderer, "images/back1.png");
    if (!back1_texture) {
        cerr << "Khong the tai back1.png! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Texture* player_jump_texture = IMG_LoadTexture(renderer, "images/jump.png");
    SDL_Texture* player_run_texture = IMG_LoadTexture(renderer, "images/run.png");
    SDL_Texture* player_stop_texture = IMG_LoadTexture(renderer, "images/stop.png");
    SDL_Texture* fireball_texture = IMG_LoadTexture(renderer, "images/fireball.png");
    if (!fireball_texture) {
        cerr << "Khong the tai fireball.png! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Texture* heart_texture = IMG_LoadTexture(renderer, "images/tim.png");
    if (!heart_texture) {
        cerr << "Khong the tai tim.png! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Texture* star_texture = IMG_LoadTexture(renderer, "images/sao.png");
    if (!star_texture) {
        cerr << "Khong the tai sao.png! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    // Khởi tạo nhân vật
    Position playerPos = {400, 400};
    bool isJumping = false;
    bool isMovingRight = true;
    float horizontalVelocity = 0.0f;
    float verticalVelocity = 0.0f;
    const float moveSpeed = 5.0f;
    const float jumpForce = -13.0f;
    const float gravity = 0.5f;
    const float fastFallSpeed = 1.0f;

    // Quản lý mạng
    int lives = 5;
    const int heartWidth = 32;
    const int heartHeight = 32;

    // Quản lý fireball
    vector<Fireball> fireballs;
    const int fireballWidth = 50;
    const int fireballHeight = 50;
    float fireballSpeed = 3.0f;
    int spawnInterval = 120;
    int spawnTimer = 0;

    // Quản lý ngôi sao
    vector<Star> stars;
    const int starWidth = 64;
    const int starHeight = 64;
    const float starFallSpeed = 2.0f;
    const int starSpawnInterval = 300;
    int starSpawnTimer = 0;

    // Quản lý thời gian đếm ngược
    int countdownTime = 30 * 60;
    bool razyMode = false;

    // Quản lý điểm số
    int score = 0;
    int highscore = loadHighscore();

    // Màu chữ
    SDL_Color white = {255, 255, 255, 255}; // Màu trắng

// Hiển thị back0 và chờ phím Space
SDL_RenderClear(renderer);
SDL_RenderCopy(renderer, back0_texture, NULL, NULL);

// Vẽ chữ "Ấn Space để bắt đầu"
string startText = " Press SPACE to start ";
SDL_Surface* startSurface = TTF_RenderText_Blended(font, startText.c_str(), white);
if (!startSurface) {
    cerr << "Khong the tao surface cho text! TTF_Error: " << TTF_GetError() << endl;
    return 1;
}
SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
if (!startTexture) {
    cerr << "Khong the tao texture cho text! SDL_Error: " << SDL_GetError() << endl;
    SDL_FreeSurface(startSurface);
    return 1;
}
SDL_Rect startRect = {(1024 - startSurface->w) / 2, 576 - startSurface->h - 135, startSurface->w, startSurface->h}; // Căn giữa màn hình
SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
SDL_RenderPresent(renderer);

// Giải phóng tài nguyên của text
SDL_FreeSurface(startSurface);
SDL_DestroyTexture(startTexture);

SDL_Event startEvent;
bool gameStarted = false;

    while (!gameStarted) {
        while (SDL_PollEvent(&startEvent)) {
            if (startEvent.type == SDL_KEYDOWN && startEvent.key.keysym.sym == SDLK_SPACE) {
                gameStarted = true;
            }
            if (startEvent.type == SDL_QUIT) {
                // Thoát game nếu người dùng đóng cửa sổ
                SDL_DestroyTexture(back0_texture);
                SDL_DestroyTexture(back1_texture);
                SDL_DestroyTexture(player_jump_texture);
                SDL_DestroyTexture(player_run_texture);
                SDL_DestroyTexture(player_stop_texture);
                SDL_DestroyTexture(fireball_texture);
                SDL_DestroyTexture(heart_texture);
                SDL_DestroyTexture(star_texture);
                TTF_CloseFont(font);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_Quit();
                IMG_Quit();
                SDL_Quit();
                return 0;
            }
        }
    }

    // Vòng lặp chính
    SDL_Event e;
    bool quit = false;
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    const int frameDelay = 1000 / 60;

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        // Xử lý sự kiện
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_w && !isJumping) {
                    isJumping = true;
                    verticalVelocity = jumpForce;
                }
            }
        }

        // Điều khiển di chuyển ngang
        horizontalVelocity = 0.0f;
        if (currentKeyStates[SDL_SCANCODE_A]) {
            horizontalVelocity = -moveSpeed;
            isMovingRight = false;
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            horizontalVelocity = moveSpeed;
            isMovingRight = true;
        }

        // Cập nhật vị trí ngang
        playerPos.x += static_cast<int>(horizontalVelocity);
        if (playerPos.x < 0) playerPos.x = 0;
        if (playerPos.x > 1024 - 64) playerPos.x = 1024 - 64;

        // Cập nhật nhảy
        if (isJumping) {
            playerPos.y += static_cast<int>(verticalVelocity);
            verticalVelocity += gravity;
            if (currentKeyStates[SDL_SCANCODE_S]) {
                verticalVelocity += fastFallSpeed;
            }
            if (playerPos.y >= 400) {
                playerPos.y = 400;
                isJumping = false;
                verticalVelocity = 0.0f;
            }
        }

        // Đếm ngược thời gian
        if (!razyMode) {
            countdownTime--;
            if (countdownTime <= 0) {
                razyMode = true;
                fireballSpeed = 5.0f;
                spawnInterval = 60;
            }
        }

        // Tạo fireball
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            Fireball fireball;
            fireball.pos.x = rand() % (1024 - fireballWidth);
            fireball.pos.y = -fireballHeight;
            fireball.active = true;
            fireball.scored = false;

            float dx = playerPos.x - fireball.pos.x;
            float dy = playerPos.y - fireball.pos.y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance > 0) {
                fireball.velocityX = (dx / distance) * fireballSpeed;
                fireball.velocityY = (dy / distance) * fireballSpeed;
            } else {
                fireball.velocityX = 0;
                fireball.velocityY = fireballSpeed;
            }

            fireballs.push_back(fireball);
            spawnTimer = 0;
        }

        // Cập nhật fireball
        for (int i = 0; i < fireballs.size(); i++) {
            if (!fireballs[i].active) continue;

            fireballs[i].pos.x += static_cast<int>(fireballs[i].velocityX);
            fireballs[i].pos.y += static_cast<int>(fireballs[i].velocityY);

            SDL_Rect fireballRect = {fireballs[i].pos.x, fireballs[i].pos.y, fireballWidth, fireballHeight};
            SDL_Rect playerRect = {playerPos.x, playerPos.y, 64, 64};
            if (SDL_HasIntersection(&fireballRect, &playerRect)) {
                fireballs[i].active = false;
                lives--;
                if (lives <= 0) {
                    quit = true;
                }
            }

            if (!fireballs[i].scored && fireballs[i].pos.y > 576) {
                score++;
                fireballs[i].scored = true;
                if (score > highscore) {
                    highscore = score;
                }
            }

            if (fireballs[i].pos.y > 576) {
                fireballs[i].active = false;
            }
        }

        fireballs.erase(
            remove_if(fireballs.begin(), fireballs.end(), [](const Fireball& f) { return !f.active; }),
            fireballs.end()
        );

        // Tạo ngôi sao
        starSpawnTimer++;
        if (!razyMode && starSpawnTimer >= starSpawnInterval) {
            Star star;
            star.pos.x = rand() % (1024 - starWidth);
            star.pos.y = -starHeight;
            star.velocityY = starFallSpeed;
            star.active = true;
            stars.push_back(star);
            starSpawnTimer = 0;
        }

        // Cập nhật ngôi sao
        for (int i = 0; i < stars.size(); i++) {
            if (!stars[i].active) continue;

            stars[i].pos.y += static_cast<int>(stars[i].velocityY);

            SDL_Rect starRect = {stars[i].pos.x, stars[i].pos.y, starWidth, starHeight};
            SDL_Rect playerRect = {playerPos.x, playerPos.y, 64, 64};
            if (SDL_HasIntersection(&starRect, &playerRect)) {
                stars[i].active = false;
                countdownTime += 5 * 60;
                if (countdownTime > 30 * 60) countdownTime = 30 * 60;
            }

            if (stars[i].pos.y > 576) {
                stars[i].active = false;
            }
        }

        stars.erase(
            remove_if(stars.begin(), stars.end(), [](const Star& s) { return !s.active; }),
            stars.end()
        );

        // Vẽ màn hình
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, back1_texture, NULL, NULL);

        // Vẽ fireball
        for (const auto& fireball : fireballs) {
            if (fireball.active) {
                SDL_Rect fireballRect = {fireball.pos.x, fireball.pos.y, fireballWidth, fireballHeight};
                SDL_RenderCopy(renderer, fireball_texture, NULL, &fireballRect);
            }
        }

        // Vẽ ngôi sao
        for (const auto& star : stars) {
            if (star.active) {
                SDL_Rect starRect = {star.pos.x, star.pos.y, starWidth, starHeight};
                SDL_RenderCopy(renderer, star_texture, NULL, &starRect);
            }
        }

        // Vẽ nhân vật
        SDL_Rect playerRect = {playerPos.x, playerPos.y, 64, 64};
        if (horizontalVelocity != 0) {
            SDL_RenderCopyEx(renderer, player_run_texture, NULL, &playerRect, 0, NULL, isMovingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
        } else if (isJumping) {
            SDL_RenderCopy(renderer, player_jump_texture, NULL, &playerRect);
        } else {
            SDL_RenderCopyEx(renderer, player_stop_texture, NULL, &playerRect, 0, NULL, isMovingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
        }

        // Vẽ số mạng (tim)
        for (int i = 0; i < lives; i++) {
            SDL_Rect heartRect = {10 + i * (heartWidth + 5), 10, heartWidth, heartHeight};
            SDL_RenderCopy(renderer, heart_texture, NULL, &heartRect);
        }

        // Vẽ score
        string scoreText = "Score: " + to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, scoreText.c_str(), white);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect = {10, 50, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        // Vẽ highscore
        string highscoreText = "Highscore: " + to_string(highscore);
        SDL_Surface* highscoreSurface = TTF_RenderText_Blended(font, highscoreText.c_str(), white);
        SDL_Texture* highscoreTexture = SDL_CreateTextureFromSurface(renderer, highscoreSurface);
        SDL_Rect highscoreRect = {10, 80, highscoreSurface->w, highscoreSurface->h};
        SDL_RenderCopy(renderer, highscoreTexture, NULL, &highscoreRect);
        SDL_FreeSurface(highscoreSurface);
        SDL_DestroyTexture(highscoreTexture);

        SDL_RenderPresent(renderer);

        // Giới hạn 60 FPS
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    // Lưu highscore vào file khi game kết thúc
    saveHighscore(highscore);

    // Giải phóng tài nguyên
    SDL_DestroyTexture(back0_texture);
    SDL_DestroyTexture(back1_texture);
    SDL_DestroyTexture(player_jump_texture);
    SDL_DestroyTexture(player_run_texture);
    SDL_DestroyTexture(player_stop_texture);
    SDL_DestroyTexture(fireball_texture);
    SDL_DestroyTexture(heart_texture);
    SDL_DestroyTexture(star_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

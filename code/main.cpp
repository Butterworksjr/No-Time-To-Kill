// TODO(evan): Make a wiki page for compiling raylib with Clang
// https://github.com/raysan5/raylib/wiki/Working-on-Windows

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define global_variable static

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
global_variable Texture2D BackgroundTexture;

global_variable int Playing;
global_variable int InMenu = 1;

enum player_states
{
    Idle,
    Run,
    JumpStart,
    JumpUp,
    FallDown
};

struct platform
{
    Vector2 Position;
    Vector2 Dimensions;
};

#define MAX_PLATFORMS 3
#define PLATFORM_WIDTH 100
#define PLATFORM_HEIGHT 10
global_variable platform Platforms[MAX_PLATFORMS];
global_variable Texture2D PlatformTexture;

#define PLAYER_WIDTH 25
#define PLAYER_HEIGHT 25
#define PLAYER_SPEED 500
#define GRAVITY 4300
#define JUMP_ACCELERATION -1300
#define JUMP_VELOCITY_DAMPEN 5
#define ROUND_TIME 90
global_variable Vector2 PlayerPosition;
global_variable Vector2 PlayerVelocity;
global_variable player_states PlayerState;
global_variable int LastPlayerDirection = 1;
global_variable int PlayerHealth = 2;
global_variable int MaxPlayerHealth = 2;
global_variable int PlayerDamage = 1;
global_variable float PlayerGold;
global_variable float GoldModifier = 1;
global_variable Texture2D PlayerIdleTexture;
global_variable Texture2D PlayerFallTexture;
global_variable Texture2D PlayerRunLeftTexture;
global_variable Texture2D PlayerRunRightTexture;
global_variable Texture2D PlayerCurrentTexture;
global_variable float TimeToDeathCounter;

global_variable float NewTime;
global_variable float OldTime;
global_variable float DeltaTime;

#define FLOOR_WIDTH WINDOW_WIDTH
#define FLOOR_HEIGHT 15
global_variable Vector2 FloorPosition;

#define DOOR_WIDTH 10
#define DOOR_HEIGHT 75

// TODO(evan): Get rid of dimensions for everything,
// just use macros
struct bullet
{
    Vector2 Position;
    Vector2 Dimensions;
    int Direction;
    
    bullet(Vector2 Position,
           Vector2 Dimensions,
           int Direction)
        : Position(Position), Dimensions(Dimensions), Direction(Direction)
    {}
};

#define BULLET_WIDTH 10
#define BULLET_HEIGHT 5
#define BULLET_SPEED 1400
#define TIME_BETWEEN_SHOTS 0.2f
global_variable std::vector<bullet> Bullets;
global_variable float ShotCounter;
global_variable Texture2D BulletTextureRight;
global_variable Texture2D BulletTextureLeft;

struct enemy
{
    Vector2 Position;
    float Health;
    
    enemy(Vector2 Position,
          float Health)
        : Position(Position), Health(Health)
    {}
};

#define ENEMY_WIDTH 25
#define ENEMY_HEIGHT 25
#define TIME_BETWEEN_ENEMY_ATTACKS 1
global_variable std::vector<enemy> Enemies;
global_variable float EnemySpawnCounter;
global_variable float AttackCounter;
global_variable int EnemySpeed = 140;
global_variable float EnemyMaxHealth = 5;
global_variable float EnemySpawnTime = 1;

int main(void)
{
    srand(time(0));
    EnemySpawnCounter = EnemySpawnTime;
    AttackCounter = TIME_BETWEEN_ENEMY_ATTACKS;
    
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "No Time To Kill");
    SetTargetFPS(60);
    
    {
        Image PlatformImage = LoadImage("../art/platform.png");
        PlatformTexture = LoadTextureFromImage(PlatformImage);
        UnloadImage(PlatformImage);
        
        Image BackgroundImage = LoadImage("../art/background.png");
        BackgroundTexture = LoadTextureFromImage(BackgroundImage);
        UnloadImage(BackgroundImage);
        
        Image BulletImage = LoadImage("../art/bullet_right.png");
        BulletTextureRight = LoadTextureFromImage(BulletImage);
        UnloadImage(BulletImage);
        BulletImage = LoadImage("../art/bullet_left.png");
        BulletTextureLeft = LoadTextureFromImage(BulletImage);
        UnloadImage(BulletImage);
        
        Image PlayerImages = LoadImage("../art/player_idle.png");
        PlayerIdleTexture = LoadTextureFromImage(PlayerImages);
        UnloadImage(PlayerImages);
        PlayerImages = LoadImage("../art/player_run_left.png");
        PlayerRunLeftTexture = LoadTextureFromImage(PlayerImages);
        UnloadImage(PlayerImages);
        PlayerImages = LoadImage("../art/player_run_right.png");
        PlayerRunRightTexture = LoadTextureFromImage(PlayerImages);
        UnloadImage(PlayerImages);
        PlayerImages = LoadImage("../art/player_fall.png");
        PlayerFallTexture = LoadTextureFromImage(PlayerImages);
        UnloadImage(PlayerImages);
    }
    
    Platforms[0].Position.x = 50;
    Platforms[0].Position.y = 300;
    Platforms[0].Dimensions.x = PLATFORM_WIDTH;
    Platforms[0].Dimensions.y = PLATFORM_HEIGHT;
    
    Platforms[1].Position.x = WINDOW_WIDTH - (PLATFORM_WIDTH + 50);
    Platforms[1].Position.y = 300;
    Platforms[1].Dimensions.x = PLATFORM_WIDTH;
    Platforms[1].Dimensions.y = PLATFORM_HEIGHT;
    
    Platforms[2].Position.x = WINDOW_WIDTH / 2 - PLATFORM_WIDTH / 2;
    Platforms[2].Position.y = 200;
    Platforms[2].Dimensions.x = PLATFORM_WIDTH;
    Platforms[2].Dimensions.y = PLATFORM_HEIGHT;
    
    FloorPosition.x = 0;
    FloorPosition.y = WINDOW_HEIGHT - FLOOR_HEIGHT;
    PlayerPosition.x = (WINDOW_WIDTH / 2) - (PLAYER_WIDTH / 2);
    PlayerPosition.y = FloorPosition.y - PLAYER_HEIGHT;
    
    while(!WindowShouldClose())
    {
        if(InMenu)
        {
            ClearBackground(RAYWHITE);
            BeginDrawing();
            
            DrawText("Made With raylib", 0, 0, 20, DARKGRAY);
            
            int TitleWidth = MeasureText("No Time To Kill", 30);
            DrawText("No Time To Kill", (WINDOW_WIDTH / 2) - (TitleWidth / 2), 50, 30, BLACK);
            
            int TimeTextWidth = MeasureText("You Have 90 Seconds To Kill Enemies", 20);
            DrawText("You Have 90 Seconds To Kill Enemies", (WINDOW_WIDTH / 2) - (TimeTextWidth / 2), 100, 20, BLACK);
            
            int MoveControlsWidth = MeasureText("W,A,S or Right,Left,Up To Move", 20);
            DrawText("W,A,S or Right,Left,Up To Move", (WINDOW_WIDTH / 2) - (MoveControlsWidth / 2), 300, 20, BLACK);
            int ShootControlsWidth = MeasureText("F or LMB To Shoot", 20);
            DrawText("F or LMB To Shoot", (WINDOW_WIDTH / 2) - (ShootControlsWidth / 2), 340, 20, BLACK);
            
            Rectangle PlayButtonRect = {(WINDOW_WIDTH / 2) - 50, 200, 100, 20};
            DrawRectangle(PlayButtonRect.x, PlayButtonRect.y,
                          PlayButtonRect.width, PlayButtonRect.height,\
                          DARKGRAY);
            Vector2 PlayTextSize = MeasureTextEx(GetFontDefault(), "PLAY", 20, 2);
            DrawText("PLAY",
                     (WINDOW_WIDTH / 2) - (PlayTextSize.x / 2),
                     210 - PlayTextSize.y / 2,
                     20, LIGHTGRAY);
            
            Vector2 MousePosition = GetMousePosition();
            Rectangle MouseHitbox = {MousePosition.x, MousePosition.y, 5, 5};
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(MouseHitbox, PlayButtonRect))
            {
                InMenu = 0;
                Playing = 1;
            }
        }
        else if(Playing)
        {
            if(TimeToDeathCounter >= ROUND_TIME)
            {
                InMenu = 0;
                Playing = 0;
            }
            
            if(EnemySpawnCounter >= EnemySpawnTime)
            {
                EnemySpawnCounter = 0;
                int SpawnDoor = rand() % 2;
                Vector2 EnemyPosition = {(float)(SpawnDoor ? DOOR_WIDTH : WINDOW_WIDTH - DOOR_WIDTH - ENEMY_WIDTH), FloorPosition.y - ENEMY_HEIGHT};
                Enemies.emplace_back(EnemyPosition, EnemyMaxHealth);
            }
            
            float RunDirection = 0;
            if((IsKeyDown(KEY_RIGHT) && IsKeyDown(KEY_LEFT)) ||
               (IsKeyDown(KEY_D) && IsKeyDown(KEY_A))) {}
            else if(IsKeyDown(KEY_RIGHT) ||
                    IsKeyDown(KEY_D)) {RunDirection = 1;}
            else if(IsKeyDown(KEY_LEFT) ||
                    IsKeyDown(KEY_A)) {RunDirection = -1;}
            
            int TryingJump =
                IsKeyDown(KEY_SPACE) ||
                IsKeyDown(KEY_UP) ||
                IsKeyDown(KEY_W) ||
                IsKeyDown(KEY_Z);
            switch(PlayerState)
            {
                case Idle:
                {
                    PlayerCurrentTexture = PlayerIdleTexture;
                    if(TryingJump)
                    {
                        PlayerState = JumpStart;
                        PlayerVelocity.y = JUMP_ACCELERATION;
                        PlayerVelocity.x = 0;
                    }
                    else if(RunDirection != 0)
                    {
                        PlayerState = Run;
                    }
                    else
                    {
                        PlayerVelocity.x = 0;
                        PlayerVelocity.y = 0;
                    }
                } break;
                
                case Run:
                {
                    if(RunDirection == 1) {PlayerCurrentTexture = PlayerRunRightTexture;}
                    else if(RunDirection == -1) {PlayerCurrentTexture = PlayerRunLeftTexture;}
                    
                    if(TryingJump)
                    {
                        PlayerState = JumpStart;
                        PlayerVelocity.y = JUMP_ACCELERATION;
                        PlayerVelocity.x *= JUMP_VELOCITY_DAMPEN;
                    }
                    else if(RunDirection == 0)
                    {
                        PlayerState = Idle;
                    }
                    else
                    {
                        PlayerVelocity.x = RunDirection * PLAYER_SPEED;
                        PlayerVelocity.y = 0;
                    }
                } break;
                
                case JumpStart:
                {
                    if(PlayerVelocity.y >= 0)
                    {
                        PlayerState = FallDown;
                    }
                    else
                    {
                        PlayerState = JumpUp;
                    }
                } break;
                
                case JumpUp:
                {
                    if(PlayerVelocity.y >= 0)
                    {
                        PlayerState = FallDown;
                    }
                    PlayerVelocity.x = RunDirection * PLAYER_SPEED;
                } break;
                
                case FallDown:
                {
                    PlayerCurrentTexture = PlayerFallTexture;
                    
                    PlayerVelocity.x = RunDirection * PLAYER_SPEED;
                } break;
            }
            
            PlayerVelocity.y += GRAVITY * DeltaTime;
            Vector2 NewPlayerPosition = Vector2Add(PlayerPosition,
                                                   Vector2Scale(PlayerVelocity, DeltaTime));
            
            if(NewPlayerPosition.y + PLAYER_HEIGHT > FloorPosition.y)
            {
                NewPlayerPosition.y = FloorPosition.y - PLAYER_HEIGHT;
                PlayerVelocity.y = 0;
                if(PlayerState == FallDown)
                {
                    PlayerState = Idle;
                }
            }
            
            for(int PlatformIndex = 0;
                PlatformIndex < MAX_PLATFORMS;
                ++PlatformIndex)
            {
                platform *Platform = &Platforms[PlatformIndex];
                if(NewPlayerPosition.y + PLAYER_HEIGHT > Platform->Position.y &&
                   NewPlayerPosition.y < Platform->Position.y)
                {
                    if(NewPlayerPosition.x + PLAYER_WIDTH > Platform->Position.x &&
                       NewPlayerPosition.x < Platform->Position.x + Platform->Dimensions.x)
                    {
                        if(PlayerPosition.y < NewPlayerPosition.y)
                        {
                            if(PlayerState == FallDown)
                            {
                                PlayerVelocity.x = 0;
                                PlayerVelocity.y = 0;
                                NewPlayerPosition.y = Platform->Position.y - PLAYER_HEIGHT;
                                PlayerState = Idle;
                            }
                            else
                            {
                                PlayerVelocity.y = 0;
                                NewPlayerPosition.y = Platform->Position.y - PLAYER_HEIGHT;
                            }
                        }
                    }
                }
            }
            
            if(PlayerState != FallDown && PlayerVelocity.y > 0)
            {
                PlayerState = FallDown;
            }
            
            if(NewPlayerPosition.x < 0)
            {
                NewPlayerPosition.x = 0;
            }
            else if(NewPlayerPosition.x > WINDOW_WIDTH - PLAYER_WIDTH - 0)
            {
                NewPlayerPosition.x = WINDOW_WIDTH - PLAYER_WIDTH - 0;
            }
            
            PlayerPosition = NewPlayerPosition;
            
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
               IsKeyDown(KEY_X) ||
               IsKeyDown(KEY_F))
            {
                if(ShotCounter >= TIME_BETWEEN_SHOTS)
                {
                    ShotCounter = 0;
                    
                    if(LastPlayerDirection == 1)
                    {
                        Vector2 BulletPosition = {PlayerPosition.x + (PLAYER_WIDTH + 5), PlayerPosition.y + (PLAYER_HEIGHT / 2) - (BULLET_HEIGHT / 2)};
                        Vector2 BulletDimensions = {BULLET_WIDTH, BULLET_HEIGHT};
                        Bullets.emplace_back(BulletPosition, BulletDimensions, LastPlayerDirection);
                    }
                    else if(LastPlayerDirection == -1)
                    {
                        Vector2 BulletPosition = {PlayerPosition.x - (PLAYER_WIDTH + 5), PlayerPosition.y + (PLAYER_HEIGHT / 2) - (BULLET_HEIGHT / 2)};
                        Vector2 BulletDimensions = {BULLET_WIDTH, BULLET_HEIGHT};
                        Bullets.emplace_back(BulletPosition, BulletDimensions, LastPlayerDirection);
                    }
                }
            }
            
            if(RunDirection)
            {
                LastPlayerDirection = RunDirection;
            }
            
            // TODO(evan): Get this to work
            for(int EnemyIndex = 0;
                EnemyIndex < Enemies.size();
                ++EnemyIndex)
            {
                enemy *Enemy = &Enemies[EnemyIndex];
                
                if(Enemy->Position.x + ENEMY_WIDTH < PlayerPosition.x)
                {
                    Enemy->Position.x = Enemy->Position.x + EnemySpeed * DeltaTime;
                }
                else //(Enemy->Position.x > PlayerPosition.x + PLAYER_WIDTH)
                {
                    Enemy->Position.x = Enemy->Position.x - EnemySpeed * DeltaTime;
                }
                
                enemy *FirstEnemy = &Enemies[EnemyIndex];
                for(int SecondEnemyIndex = 0;
                    SecondEnemyIndex < Enemies.size();
                    ++SecondEnemyIndex)
                {
                    if(EnemyIndex == SecondEnemyIndex) {continue;}
                    
                    enemy *SecondEnemy = &Enemies[SecondEnemyIndex];
                    if(FirstEnemy->Position.x + ENEMY_WIDTH > SecondEnemy->Position.x &&
                       FirstEnemy->Position.x + ENEMY_WIDTH < SecondEnemy->Position.x + ENEMY_WIDTH)
                    {
                        FirstEnemy->Position.x = SecondEnemy->Position.x - ENEMY_WIDTH;
                    }
                    else if(FirstEnemy->Position.x < SecondEnemy->Position.x + ENEMY_WIDTH &&
                            FirstEnemy->Position.x > SecondEnemy->Position.x)
                    {
                        FirstEnemy->Position.x = SecondEnemy->Position.x + ENEMY_WIDTH;
                    }
                }
                
                Rectangle EnemyRect = {Enemy->Position.x, Enemy->Position.y, ENEMY_WIDTH, ENEMY_HEIGHT};
                if(AttackCounter >= TIME_BETWEEN_ENEMY_ATTACKS)
                {
                    Rectangle PlayerRect = {PlayerPosition.x, PlayerPosition.y, PLAYER_WIDTH, PLAYER_HEIGHT};
                    if(CheckCollisionRecs(EnemyRect, PlayerRect))
                    {
                        AttackCounter = 0;
                        PlayerHealth -= 1;
                    }
                }
                
                for(int BulletIndex = 0;
                    BulletIndex < Bullets.size();
                    ++BulletIndex)
                {
                    bullet *Bullet = &Bullets[BulletIndex];
                    Rectangle BulletRect = {Bullet->Position.x, Bullet->Position.y, BULLET_WIDTH, BULLET_HEIGHT};
                    
                    if(CheckCollisionRecs(EnemyRect, BulletRect))
                    {
                        Enemy->Health -= PlayerDamage;
                        if(Enemy->Health <= 0)
                        {
                            Enemies.erase(Enemies.begin() + EnemyIndex);
                            EnemyIndex -= 1;
                            PlayerGold += GoldModifier;
                        }
                        Bullets.erase(Bullets.begin() + BulletIndex);
                        BulletIndex -= 1;
                    }
                }
            }
            
            if(PlayerHealth <= 0)
            {
                InMenu = 0;
                Playing = 0;
            }
            
            DrawTexture(BackgroundTexture, 0, 0, WHITE);
            DrawRectangle(0, FloorPosition.y - DOOR_HEIGHT,
                          DOOR_WIDTH, DOOR_HEIGHT, DARKGRAY);
            DrawRectangle(WINDOW_WIDTH - DOOR_WIDTH, FloorPosition.y - DOOR_HEIGHT,
                          DOOR_WIDTH, DOOR_HEIGHT, DARKGRAY);
            for(int PlatformIndex = 0;
                PlatformIndex < MAX_PLATFORMS;
                ++PlatformIndex)
            {
                platform *Platform = &Platforms[PlatformIndex];
                DrawTexture(PlatformTexture,
                            Platform->Position.x,
                            Platform->Position.y,
                            WHITE);
            }
            
            for(int BulletIndex = 0;
                BulletIndex < Bullets.size();
                ++BulletIndex)
            {
                bullet *Bullet = &Bullets[BulletIndex];
                
                Bullet->Position.x += BULLET_SPEED * Bullet->Direction * DeltaTime;
                
                if(Bullet->Position.x > WINDOW_WIDTH ||
                   Bullet->Position.x + BULLET_WIDTH < 0)
                {
                    Bullets.erase(Bullets.begin() + BulletIndex);
                    break;
                }
                
                if(LastPlayerDirection == -1)
                {
                    DrawTexture(BulletTextureLeft,
                                Bullet->Position.x, Bullet->Position.y,
                                WHITE);
                }
                else
                {
                    DrawTexture(BulletTextureRight,
                                Bullet->Position.x, Bullet->Position.y,
                                WHITE);
                }
            }
            
            for(int EnemyIndex = 0;
                EnemyIndex < Enemies.size();
                ++EnemyIndex)
            {
                enemy *Enemy = &Enemies[EnemyIndex];
                
                DrawRectangle(Enemy->Position.x, Enemy->Position.y,
                              ENEMY_WIDTH, ENEMY_HEIGHT, DARKGREEN);
            }
            
            DrawTexture(PlayerCurrentTexture,
                        PlayerPosition.x, PlayerPosition.y,
                        WHITE);
            DrawText(TextFormat("Health: %d", PlayerHealth), 0, 0, 20, LIGHTGRAY);
            
            DeltaTime = GetFrameTime();
            ShotCounter += DeltaTime;
            EnemySpawnCounter += DeltaTime;
            AttackCounter += DeltaTime;
            TimeToDeathCounter += DeltaTime;
        }
        else // NOTE(evan): We are in the shop
        {
            ClearBackground(RAYWHITE);
            
            Vector2 MousePosition = GetMousePosition();
            Rectangle MouseHitbox = {MousePosition.x, MousePosition.y, 5, 5};
            
            int ShopTextWidth = MeasureText("SHOP: -10 Gold", 30);
            DrawText("SHOP: -10 Gold", (WINDOW_WIDTH / 2) - (ShopTextWidth / 2), 50, 30, BLACK);
            DrawText(TextFormat("Gold: %.2f", PlayerGold), 0, 100, 20, BLACK);
            
            Rectangle HealthUpgradeRect = {(WINDOW_WIDTH / 2) - 50, 200, 100, 20};
            DrawRectangle(HealthUpgradeRect.x, HealthUpgradeRect.y,
                          HealthUpgradeRect.width, HealthUpgradeRect.height,
                          DARKGRAY);
            Vector2 HealthUpgradeTextSize = MeasureTextEx(GetFontDefault(), "+1 Health", 20, 2);
            DrawText("+1 Health", (WINDOW_WIDTH / 2) - (HealthUpgradeTextSize.x / 2), 210 - HealthUpgradeTextSize.y / 2, 20, LIGHTGRAY);
            
            Rectangle GoldUpgradeRect = {(WINDOW_WIDTH / 2) - 87.5, 250, 175, 20};
            DrawRectangle(GoldUpgradeRect.x, GoldUpgradeRect.y,
                          GoldUpgradeRect.width, GoldUpgradeRect.height,
                          DARKGRAY);
            Vector2 GoldUpgradeTextSize = MeasureTextEx(GetFontDefault(), "+1 Gold Modifier", 20, 2);
            DrawText("+1 Gold Modifier", (WINDOW_WIDTH / 2) - (GoldUpgradeTextSize.x / 2), 260 - GoldUpgradeTextSize.y / 2, 20, LIGHTGRAY);
            
            Rectangle DamageUpgradeRect = {(WINDOW_WIDTH / 2) - 50, 300, 100, 20};
            DrawRectangle(DamageUpgradeRect.x, DamageUpgradeRect.y,
                          DamageUpgradeRect.width, DamageUpgradeRect.height,
                          DARKGRAY);
            Vector2 DamageUpgradeTextSize = MeasureTextEx(GetFontDefault(), "+1 Damage", 20, 2);
            DrawText("+1 Damage", (WINDOW_WIDTH / 2) - (DamageUpgradeTextSize.x / 2), 310 - DamageUpgradeTextSize.y / 2, 20, LIGHTGRAY);
            
            Rectangle PlayButtonRect = {(WINDOW_WIDTH / 2) - 50, 400, 100, 20};
            DrawRectangle(PlayButtonRect.x, PlayButtonRect.y,
                          PlayButtonRect.width, PlayButtonRect.height,
                          DARKGRAY);
            Vector2 PlayTextSize = MeasureTextEx(GetFontDefault(), "PLAY", 20, 2);
            DrawText("PLAY",
                     (WINDOW_WIDTH / 2) - (PlayTextSize.x / 2),
                     410- PlayTextSize.y / 2,
                     20, LIGHTGRAY);
            
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                if(CheckCollisionRecs(MouseHitbox, PlayButtonRect))
                {
                    InMenu = 0;
                    Playing = 1;
                }
                else if(CheckCollisionRecs(MouseHitbox, HealthUpgradeRect))
                {
                    if(PlayerGold >= 10)
                    {
                        PlayerGold -= 10;
                        MaxPlayerHealth += 1;
                        EnemyMaxHealth += 1.5f;
                    }
                }
                else if(CheckCollisionRecs(MouseHitbox, GoldUpgradeRect))
                {
                    if(PlayerGold >= 10)
                    {
                        PlayerGold -= 10;
                        GoldModifier += 1;
                        EnemyMaxHealth += 1;
                    }
                }
                else if(CheckCollisionRecs(MouseHitbox, DamageUpgradeRect))
                {
                    if(PlayerGold >= 10)
                    {
                        PlayerGold -= 10;
                        PlayerDamage += 1;
                        EnemySpeed += 75;
                        EnemyMaxHealth += 0.5f;
                        if(EnemySpawnTime > 0.4f)
                        {
                            EnemySpawnTime -= 0.15f;
                        }
                    }
                }
            }
            
            Enemies.clear();
            PlayerHealth = MaxPlayerHealth;
            TimeToDeathCounter = ROUND_TIME;
        }
        
        EndDrawing();
        printf("FPS: %d\n", GetFPS());
    }
    
    UnloadTexture(PlatformTexture);
    UnloadTexture(BackgroundTexture);
    UnloadTexture(BulletTextureRight);
    UnloadTexture(BulletTextureLeft);
    UnloadTexture(PlayerIdleTexture);
    UnloadTexture(PlayerRunLeftTexture);
    UnloadTexture(PlayerRunRightTexture);
    UnloadTexture(PlayerFallTexture);
    CloseWindow();
    return(0);
}

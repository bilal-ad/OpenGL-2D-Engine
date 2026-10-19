#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "player_object.h"

#include <algorithm>
#include <iostream>
#include <vector>


SpriteRenderer* Renderer;
PlayerObject* Player;
GameObject* WinScreen;
GameObject* StartScreen;

glm::vec2 PlayerStartPos;

const glm::vec2 PLAYER_SIZE(70.0f, 90.0f);
const float PLAYER_SPEED = 300.0f;
const float JUMP_FORCE = 600.0f;
const float GRAVITY = 1200.0f;


bool CheckCollision(GameObject& one, GameObject& two);
bool CheckCollisionPlatform(GameObject& one, GameObject& platform);


Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0)
{

}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete WinScreen;
    delete StartScreen;
}


void Game::Init()
{
   
    ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
    
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
  
    ResourceManager::LoadTexture("textures/background.png", false, "background");
    ResourceManager::LoadTexture("textures/platform.png", true, "platform");
    ResourceManager::LoadTexture("textures/player.png", true, "player");
    ResourceManager::LoadTexture("textures/goal.png", true, "goal");
    ResourceManager::LoadTexture("textures/win.png", true, "win");
    ResourceManager::LoadTexture("textures/start.png", true, "start");
    ResourceManager::LoadTexture("textures/spike.png", true, "hazard");
   
    GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height);
    GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Level = 0;

    if (this->Levels[this->Level].HasPlayerStart)
    {
        PlayerStartPos = this->Levels[this->Level].PlayerStartPosition;
    }
    else
    {
        PlayerStartPos = glm::vec2(100.0f, 300.0f);
    }

    StartScreen = new GameObject(
        glm::vec2(this->Width / 2.0f - 250.0f, this->Height / 2.0f - 125.0f),
        glm::vec2(500.0f, 250.0f),
        ResourceManager::GetTexture("start")
    );

    WinScreen = new GameObject(
        glm::vec2(this->Width / 2.0f - 200.0f, this->Height / 2.0f - 100.0f),
        glm::vec2(400.0f, 200.0f),
        ResourceManager::GetTexture("win")
    );

    Player = new PlayerObject(
        PlayerStartPos,
        PLAYER_SIZE,
        ResourceManager::GetTexture("player")
    );

}

void Game::Update(float dt)
{
    if (this->State != GAME_ACTIVE)
        return;

    // 1. Horizontale Bewegung
    Player->Position.x += Player->Velocity.x * dt;
    // 2. Horizontale Kollisionen auflösen
    DoCollisionsX();

    // 3. Schwerkraft wirkt auf die vertikale Geschwindigkeit
    Player->Velocity.y += GRAVITY * dt;
    // 4. Vertikale Bewegung
    Player->Position.y += Player->Velocity.y * dt;
    // 5. Vor der Y-Kollision erstmal sagen:
    // Spieler ist nicht auf dem Boden.
    Player->IsGrounded = false;
    // 6. Vertikale Kollisionen auflösen
    DoCollisionsY();

    // 7. Falls der Spieler aus der Map fällt, zurücksetzen
    if (Player->Position.y > this->Height + 200.0f)
    {
        ResetPlayer();
    }

    for (GameObject& hazard : Levels[Level].Hazards)
    {
        if (CheckCollision(*Player, hazard))
        {
            ResetPlayer();
            break;
        }
    }

    for (GameObject& goal : Levels[Level].Goals)
    {
        if (CheckCollision(*Player, goal))
        {
            if (this->Level + 1 < this->Levels.size())
            {
                // nächstes Level starten
                this->Level++;
                ResetPlayer();
            }
            else
            {
                // letztes Level geschafft
                this->State = GAME_WIN;
            }

            break;
        }
    }
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            ResetPlayer();
        }
    }

    if (this->State == GAME_ACTIVE)
    {
        Player->Velocity.x = 0.0f;

        if (this->Keys[GLFW_KEY_A])
        {
            Player->Velocity.x = -PLAYER_SPEED;
        }

        if (this->Keys[GLFW_KEY_D])
        {
            Player->Velocity.x = PLAYER_SPEED;
        }

        if (this->Keys[GLFW_KEY_SPACE])
        {
            Player->Jump(JUMP_FORCE);
        }
    }

    if (this->State == GAME_WIN)
    {
        if (this->Keys[GLFW_KEY_R])
        {
            this->Level = 0;
            this->State = GAME_ACTIVE;
            ResetPlayer();
        }
    }
    
}

void Game::Render()
{
    
    Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

    if (this->State == GAME_MENU)
    {
        StartScreen->Draw(*Renderer);
    }
    else if (this->State == GAME_ACTIVE)
    {
   
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer); 

    }
    else if (this->State == GAME_WIN)
    {
        WinScreen->Draw(*Renderer);
    }
}

void Game::DoCollisionsX()
{
    for (GameObject& box : Levels[Level].Platforms)
    {
        if (box.Destroyed)
            continue;

        if (CheckCollisionPlatform(*Player, box))
        {
            float insetX = 8.0f;

            float boxLeft = box.Position.x + insetX;
            float boxRight = box.Position.x + box.Size.x - insetX;

            if (Player->Velocity.x > 0.0f)
            {
                // Spieler läuft nach rechts gegen einen Block.
                // Also muss er links neben den Block gesetzt werden.
                Player->Position.x = box.Position.x - Player->Size.x;
            }
            else if (Player->Velocity.x < 0.0f)
            {
                // Spieler läuft nach links gegen einen Block.
                // Also muss er rechts neben den Block gesetzt werden.
                Player->Position.x = box.Position.x + box.Size.x;
            }

            // Horizontale Bewegung stoppen
            Player->Velocity.x = 0.0f;
        }
    }
}

void Game::DoCollisionsY()
{
    for (GameObject& box : Levels[Level].Platforms)
    {
        if (box.Destroyed)
            continue;

        if (CheckCollisionPlatform(*Player, box))
        {
            if (Player->Velocity.y > 0.0f)
            {
                // Spieler fällt von oben auf den Block.
                // Also wird er auf den Block gestellt.
                Player->Position.y = box.Position.y - Player->Size.y;

                // Fallbewegung stoppen
                Player->Velocity.y = 0.0f;

                // Spieler darf wieder springen
                Player->IsGrounded = true;
            }
            else if (Player->Velocity.y < 0.0f)
            {
                // Spieler springt von unten gegen den Block.
                // Also wird er unter den Block gesetzt.
                Player->Position.y = box.Position.y + box.Size.y;

                // Aufwärtsbewegung stoppen
                Player->Velocity.y = 0.0f;
            }
        }
    }
}

bool CheckCollision(GameObject& one, GameObject& two)
{
    // kollision x-axen?
    bool collisionX = one.Position.x + one.Size.x > two.Position.x &&
        two.Position.x + two.Size.x > one.Position.x;
    // kollision y-axem?
    bool collisionY = one.Position.y + one.Size.y > two.Position.y &&
        two.Position.y + two.Size.y > one.Position.y;
    // kollision nur, wenn beiden axen
    return collisionX && collisionY;
}

bool CheckCollisionPlatform(GameObject& one, GameObject& platform)
{
    float insetX = 8.0f;

    float oneLeft = one.Position.x;
    float oneRight = one.Position.x + one.Size.x;
    float oneTop = one.Position.y;
    float oneBottom = one.Position.y + one.Size.y;

    float platformLeft = platform.Position.x + insetX;
    float platformRight = platform.Position.x + platform.Size.x - insetX;
    float platformTop = platform.Position.y;
    float platformBottom = platform.Position.y + platform.Size.y;

    bool collisionX =
        oneRight > platformLeft &&
        platformRight > oneLeft;

    bool collisionY =
        oneBottom > platformTop &&
        platformBottom > oneTop;

    return collisionX && collisionY;
}

void Game::ResetPlayer()
{
    if (this->Levels[this->Level].HasPlayerStart)
    {
        PlayerStartPos = this->Levels[this->Level].PlayerStartPosition;
    }
    else
    {
        PlayerStartPos = glm::vec2(100.0f, 300.0f);
    }

    Player->Position = PlayerStartPos;
    Player->Velocity = glm::vec2(0.0f, 0.0f);
    Player->IsGrounded = false;
}

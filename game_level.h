#ifndef GAMELEVEL_H
#define GAMELEVEL_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"



class GameLevel
{
public:
    std::vector<GameObject> Platforms;
    std::vector<GameObject> Goals;
    std::vector<GameObject> Hazards;

    glm::vec2 PlayerStartPosition;
    bool HasPlayerStart;

    GameLevel() : PlayerStartPosition(0.0f, 0.0f), HasPlayerStart(false) { }

    void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);
  
    void Draw(SpriteRenderer &renderer);
   
    bool IsCompleted();
private:
 
    void init(std::vector<std::vector<unsigned int>> tileData, 
       unsigned int levelWidth,
        unsigned int levelHeight
    );
};

#endif
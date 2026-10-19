#include "game_level.h"
#include "resource_manager.h"

#include <fstream>
#include <sstream>


void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight)
{
  
    this->Platforms.clear();
    this->Goals.clear();
    this->Hazards.clear();

    this->PlayerStartPosition = glm::vec2(0.0f, 0.0f);
    this->HasPlayerStart = false;
    
    unsigned int tileCode;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line)) // lese jede zeile vom lvl datei
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode) 
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
            this->init(tileData, levelWidth, levelHeight);
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject& tile : this->Platforms)
    {
        if (!tile.Destroyed)
            tile.Draw(renderer);
    }

    for (GameObject& hazard : this->Hazards)
    {
        if (!hazard.Destroyed)
            hazard.Draw(renderer);
    }

    for (GameObject& goal : this->Goals)
    {
        if (!goal.Destroyed)
            goal.Draw(renderer);
    }
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Platforms)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size();

    float unit_width = levelWidth / static_cast<float>(width);
    float unit_height = levelHeight / static_cast<float>(height);


    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            glm::vec2 cellPos(unit_width * x, unit_height * y);

            float platformHeight = 45.0f;

            glm::vec2 platformPos(
                cellPos.x,
                cellPos.y + unit_height - platformHeight
            );

            glm::vec2 platformSize(
                unit_width,
                platformHeight
            );

            glm::vec2 objectSize(
                unit_width,
                unit_width
            );

            glm::vec2 objectPos(
                cellPos.x,
                cellPos.y + unit_height - objectSize.y
            );

            if (tileData[y][x] == 1) 
            {
                GameObject obj(
                    platformPos,
                    platformSize,
                    ResourceManager::GetTexture("platform"),
                    glm::vec3(1.0f)
                );
                this->Platforms.push_back(obj);
            }
            else if (tileData[y][x] == 3)
            {
                GameObject goal(
                    objectPos,
                    objectSize,
                    ResourceManager::GetTexture("goal")
                );
                this->Goals.push_back(goal);
            }
            else if (tileData[y][x] == 4)
            {
                GameObject hazard(
                    objectPos,
                    objectSize,
                    ResourceManager::GetTexture("hazard")
                );
                this->Hazards.push_back(hazard);
            }
            else if (tileData[y][x] == 5)
            {
                this->PlayerStartPosition = objectPos;
                this->HasPlayerStart = true;
            }
        }
    }
}
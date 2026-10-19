#ifndef PLAYER_OBJECT_H
#define PLAYER_OBJECT_H

#include "game_object.h"

class PlayerObject : public GameObject
{
public:
    bool IsGrounded;

    PlayerObject();
    PlayerObject(glm::vec2 position, glm::vec2 size, Texture2D sprite);

    void Update(float dt);
    void Jump(float jumpForce);
};

#endif

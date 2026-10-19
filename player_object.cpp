#include "player_object.h"

PlayerObject::PlayerObject()
    : GameObject(), IsGrounded(false)
{
}

PlayerObject::PlayerObject(glm::vec2 position, glm::vec2 size, Texture2D sprite)
    : GameObject(position, size, sprite), IsGrounded(false)
{
}

void PlayerObject::Update(float dt)
{
    this->Position += this->Velocity * dt;
}

void PlayerObject::Jump(float jumpForce)
{
    if (this->IsGrounded)
    {
        this->Velocity.y = -jumpForce;
        this->IsGrounded = false;
    }
}
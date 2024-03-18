#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
    this->mesh = mesh;
    this->material = material;
}

std::shared_ptr<Mesh> GameEntity::GetMesh() { return this->mesh; }
Transform* GameEntity::GetTransform() { return &transform; }
std::shared_ptr<Material> GameEntity::GetMaterial() { return this->material; }

void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
void GameEntity::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }
#include <memory>

#include "Mesh.h"
#include "Transform.h"
#include "Material.h"

#pragma once
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();
	std::shared_ptr<Material> GetMaterial();

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(std::shared_ptr<Material> material);

private:
	std::shared_ptr<Mesh> mesh;
	Transform transform;
	std::shared_ptr<Material> material;
};
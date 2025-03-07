#include "Framework.h"

Hermit::Hermit(string modelName) : Model(modelName)
{
	collider = new CapsuleCollider(0.5f, 3.0f);
	collider->SetParent(this);
	collider->SetLocalPosition(Vector3(0, 0, 0));
	collider->UpdateWorld();

}

Hermit::~Hermit()
{
	delete collider;
}

void Hermit::Render()
{
	Model::Render();
	collider->Render();
}

void Hermit::Update()
{
	UpdateWorld();
	if(collider)
		collider->SetLocalPosition(Vector3(0, 0, 0));
		collider->UpdateWorld();
}

void Hermit::ByeOre()
{
}

void Hermit::SellItem()
{
}

bool Hermit::IsRayCollision(const Ray& ray, RaycastHit* hit)
{
	return collider->IsRayCollision(ray, hit);
}

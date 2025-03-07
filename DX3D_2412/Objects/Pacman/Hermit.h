#pragma once

class CapsuleCollider;

class Hermit : public Model
{
public:
	Hermit(string modelName);
	~Hermit();

	void Render() ; 
	void Update() ;
	void ByeOre();
	void SellItem();
	bool IsRayCollision(const Ray& ray, RaycastHit* hit);
private:

	CapsuleCollider* collider;
	bool showCollider = true;
};
#pragma once

class Hermit;

class CubeMapEditor : public Cube
{
private:
	enum EditType
	{
		None, Wall, Coin, StartPos, Ore, Item
	};

public:
	CubeMapEditor();
	~CubeMapEditor();

	void Update();
	void Render();
	void Edit();

private:
	void SetSelectTile();

	void CreateWall();
	void CreateCoin();
	void CreateStartPos();
	void CreateOre();
	void CreatePreview();

	void DeleteObject();

	vector<CapsuleCollider*> GetCollisionCoins(float distance);
	void SetEdit();
	void CreatePlayer();
	void UpdateMesh();

	void Save();
	void Load();

private:	
	POINT mapSize;
	POINT mapTiling = { 1, 1 };
	POINT selectTile;

	EditType editType = Wall;

	FloatValueBuffer* tileBuffer;

	list<GameObject*> objects;
	map<int, GameObject*> objectData;

	list<Model*> model;
	map<int, Model*> modeldate;

	list<CapsuleCollider*> colliders;
	map<int, CapsuleCollider*> colliderdata;

	list<Hermit*> hermits;
	map<int, Hermit*> hermitdata;

	vector<EditType> types;

	map<EditType, GameObject*> previews;

	Player* player = nullptr;
};
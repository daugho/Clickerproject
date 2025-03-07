#include "Framework.h"

CubeMapEditor::CubeMapEditor()
{	
	CreatePreview();

	tileBuffer = new FloatValueBuffer();

	Load();
	material->SetShader(L"Custom/MapEditor.hlsl");	

}

CubeMapEditor::~CubeMapEditor()
{	
	delete tileBuffer;

	for (GameObject* object : objects)
		delete object;

	for (pair<EditType, GameObject*> preview : previews)
		delete preview.second;
}

void CubeMapEditor::Update()
{
	SetSelectTile();
	SetEdit();	
	if (player != nullptr)
	{
		vector<CapsuleCollider*> hitCoins = GetCollisionCoins(3.0f);

		for (CapsuleCollider* collider : colliders)
		{
			if (find(hitCoins.begin(), hitCoins.end(), collider) != hitCoins.end())
			{
				collider->SetColor(Float4(1.0f, 0.0f, 0.0f, 1.0f));
			}
			else
			{
				collider->SetColor(Float4(1, 1, 1, 1));
			}
		}
	}
	if (KEY->Down(VK_NUMPAD7))
	{
		CreatePlayer();
		CAM->SetTarget(player);
		CAM->TargetOptionLoad("FPSMode");
	}
	if (player != nullptr)
		player->Update();
	for (pair<EditType, GameObject*> preview : previews)
		preview.second->Update();
}

void CubeMapEditor::Render()
{	
	tileBuffer->SetPS(10);
	Cube::Render();

	for (GameObject* object : objects)
		object->Render();
	for (Model* mode : model)
		mode->Render();
	for(CapsuleCollider* capsul : colliders)
		capsul->Render();
	for (Hermit* hermit : hermits)
		hermit->Render();
	Environment::Get()->SetAdditive();
	for (pair<EditType, GameObject*> preview : previews)
		preview.second->Render();
	if (player != nullptr)
		player->Render();
	Environment::Get()->SetAlphaBlend(false);
}

void CubeMapEditor::Edit()
{
	ImGui::Text("MapEditor");
		
	ImGui::DragInt2("SelectTile", (int*)&selectTile);

	if (ImGui::DragInt2("MapSize", (int*)&mapSize), 1, 1, 100)
	{		
		size = Vector3(mapSize.x, 1.0f, mapSize.y);

		UpdateMesh();
	}	

	if (ImGui::DragInt2("Tiling", (int*)&mapTiling), 1, 1, 100)
	{
		tiling = mapTiling;
		UpdateMesh();
	}

	material->Edit();

	for (GameObject* object : objects)
		object->Edit();

	const char* list[] = {"None", "Wall", "Coin", "StartPos", "Ore", "Item"};
	if (ImGui::Combo("Type", (int*)&editType, list, 6))
	{
		for(pair<EditType, GameObject*> preview : previews)
		{
			if (editType == preview.first)
			{
				preview.second->SetActive(true);
			}
			else
			{
				preview.second->SetActive(false);
			}
		}
	}

	if (ImGui::Button("Save"))
	{
		Save();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{
		Load();
	}
}

void CubeMapEditor::SetSelectTile()
{
	Ray ray = CAM->ScreenPointToRay(mousePos);
	RaycastHit hit;

	if (collider->IsRayCollision(ray, &hit))
	{
		Vector2 size = mapSize;
		Vector2 pos = { hit.point.x, hit.point.z };
		pos += size * 0.5f;
		selectTile = { (int)pos.x, (int)pos.y };

		tileBuffer->Get()[0] = (float)selectTile.x / mapSize.x * mapTiling.x;
		tileBuffer->Get()[1] = (mapSize.y - (float)selectTile.y - 1) / mapSize.y * mapTiling.y;

		if (previews.count(editType) > 0)
		{
			float y = 0.0f;

			switch (editType)
			{
			case CubeMapEditor::Wall:
				y = 2.0f;
				break;
			case CubeMapEditor::Coin:
				y = 1.0f;
				break;
			case CubeMapEditor::StartPos:
				break;
			case CubeMapEditor::Ore:
				break;
			case CubeMapEditor::Item:
				break;
			default:
				break;
			}

			Vector3 startPos = Vector3(mapSize.x * -0.5f, 0.0f, mapSize.y * -0.5f);
			Vector3 pos = startPos + Vector3(selectTile.x + 0.5f, y, selectTile.y + 0.5f);
			previews[editType]->SetLocalPosition(pos);
		}
	}
}

void CubeMapEditor::CreateWall()
{
	Vector3 startPos = Vector3(mapSize.x * -0.5f, 0.0f, mapSize.y * -0.5f);
	Vector3 pos = startPos + Vector3(selectTile.x + 0.5f, 2.0f, selectTile.y + 0.5f);

	int index = selectTile.y * mapSize.x + selectTile.x;

	if (objectData.count(index) > 0)
		return;

	Cube* wall = new Cube({ 1, 3, 1 });
	wall->SetLocalPosition(pos);
	wall->GetMaterial()->SetDiffuseMap(L"Resources/Textures/Landscape/Dirt3.png");
	wall->Update();

	objects.push_back(wall);

	objectData[index] = wall;
	types[index] = Wall;
}

void CubeMapEditor::CreateCoin()
{
	Vector3 startPos = Vector3(mapSize.x * -0.5f, 0.0f, mapSize.y * -0.5f);
	Vector3 pos = startPos + Vector3(selectTile.x + 0.5f, 0.5f, selectTile.y + 0.5f);

	int index = selectTile.y * mapSize.x + selectTile.x;

	if (objectData.count(index) > 0)
		return;

	Model* coin = new Model("machine");
	coin->SetLocalPosition(pos);
	coin->UpdateWorld();

	CapsuleCollider* collider = new CapsuleCollider(0.5f, 3.0f);
	collider->SetLocalPosition(pos);
	collider->SetParent(coin);
	collider->SetLocalPosition(Vector3(0, 0, 0));
	collider->UpdateWorld();

	model.push_back(coin);
	colliders.push_back(collider);
	colliderdata[index] = collider;
	modeldate[index] = coin;
	types[index] = Coin;
}

void CubeMapEditor::CreateStartPos()
{
	Vector3 startPos = Vector3(mapSize.x * -0.5f, 0.0f, mapSize.y * -0.5f);
	Vector3 pos = startPos + Vector3(selectTile.x + 0.5f, 1.0f, selectTile.y + 0.5f);

	int index = selectTile.y * mapSize.x + selectTile.x;	

	Sphere* coin = new Sphere(0.5f);
	coin->SetLocalPosition(pos);
	coin->GetMaterial()->GetData()->diffuse = { 0, 1, 0, 1 };
	coin->UpdateWorld();

	objects.push_back(coin);

	objectData[index] = coin;
	types[index] = StartPos;
}

void CubeMapEditor::CreateOre()
{
	Vector3 startPos = Vector3(mapSize.x * -0.5f, 0.0f, mapSize.y * -0.5f);
	Vector3 pos = startPos + Vector3(selectTile.x + 0.5f, 0.5f, selectTile.y + 0.5f);

	int index = selectTile.y * mapSize.x + selectTile.x;

	if (hermitdata.count(index) > 0)
		return;

	Hermit* ore = new Hermit("MinerOre");
	ore->SetLocalPosition(pos);
	ore->Update();

	hermits.push_back(ore);

	hermitdata[index] = ore;
	types[index] = Ore;
}

void CubeMapEditor::CreatePreview()
{
	Cube* wall = new Cube({ 1, 3, 1 });
	wall->GetMaterial()->SetShader(L"Custom/Preview.hlsl");
	wall->GetMaterial()->SetDiffuseMap(L"Resources/Textures/Landscape/Dirt3.png");
	wall->GetMaterial()->GetData()->emissive = { 0, 1, 1, 0.5f };
	wall->SetActive(true);

	previews[Wall] = wall;

	Sphere* coin = new Sphere(0.5f);
	coin->GetMaterial()->SetShader(L"Custom/Preview.hlsl");	
	coin->GetMaterial()->GetData()->emissive = { 1, 1, 0, 0.5f };
	coin->SetActive(false);

	previews[Coin] = coin;
}

void CubeMapEditor::DeleteObject()
{
	int index = selectTile.y * mapSize.x + selectTile.x;

	if (objectData.count(index) == 0)
		return;

	list<GameObject*>::iterator deleteObject = find(objects.begin(), objects.end(), objectData[index]);

	objects.erase(deleteObject);
	objectData.erase(index);
}

vector<CapsuleCollider*> CubeMapEditor::GetCollisionCoins(float distance)
{
	Ray ray = CAM->ScreenPointToRay(CENTER);  // 화면 중앙에서 Ray 발사

	vector<CapsuleCollider*> hitCoins;  // 충돌한 Coin을 저장할 리스트
	RaycastHit hit;

	for (CapsuleCollider* collider : colliders)  // 모든 Coin 콜라이더 순회
	{
		float dist = Vector3::Distance(collider->GetGlobalPosition(), CAM->GetGlobalPosition());

		if (dist > distance)  // 지정한 거리보다 멀면 검사 스킵
			continue;

		if (collider->IsRayCollision(ray, &hit))  // Ray 충돌 검사
		{
			hitCoins.push_back(collider);  // 충돌한 Coin 저장
		}
	}

	return hitCoins;  // 충돌한 Coin 리스트 반환
}

void CubeMapEditor::SetEdit()
{
	if (KEY->Press(VK_LBUTTON) && !ImGui::GetIO().WantCaptureMouse)
	{
		switch (editType)
		{
		case CubeMapEditor::Wall:
			CreateWall();
			break;
		case CubeMapEditor::Coin:
			CreateCoin();
			break;
		case CubeMapEditor::StartPos:
			CreateStartPos();
			break;
		case CubeMapEditor::Ore:
			CreateOre();
			break;
		case CubeMapEditor::Item:
			break;
		default:
			break;
		}
	}

	if (KEY->Down(VK_RBUTTON))
	{
		DeleteObject();
	}
}

void CubeMapEditor::CreatePlayer()
{
	if (player != nullptr)
	{
		return;
	}

	player = new Player();
	player->SetLocalPosition(Vector3(5, 5, 5));
	player->UpdateWorld();

	//objects.push_back(player);
}

void CubeMapEditor::UpdateMesh()
{
	if (mapSize.x == 0 || mapSize.y == 0)
		return;

	SetVertices();
	MakeNormal();
	MakeTangent();
	mesh->UpdateVertices();
	collider->UpdateMesh(size);

	tileBuffer->Get()[2] = 1.0f / mapSize.x * mapTiling.x;
	tileBuffer->Get()[3] = 1.0f / mapSize.y * mapTiling.y;

	types.resize(mapSize.x * mapSize.y);
}


void CubeMapEditor::Save()
{
	BinaryWriter* writer = new BinaryWriter("Resources/TextData/Pacman.map");

	writer->Data<POINT>(mapSize);
	writer->Data<POINT>(mapTiling);

	writer->Byte(types.data(), sizeof(EditType) * types.size());

	delete writer;
}

void CubeMapEditor::Load()
{
	BinaryReader* reader = new BinaryReader("Resources/TextData/Pacman.map");

	mapSize = reader->Data<POINT>();
	mapTiling = reader->Data<POINT>();

	material->Load("Resources/Materials/PacmanMap.mat");

	UpdateMesh();

	delete reader;
}

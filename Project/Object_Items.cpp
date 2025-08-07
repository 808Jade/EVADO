#include "stdafx.h"
#include "Object_Items.h"

void Item::ChangeExistState(bool isExist)
{
	is_exist = isExist;
}

void Item::Animate(float fTimeElapsed)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Shovel::Shovel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pShovelModel = pModel;
	if (!pShovelModel) pShovelModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Shovel.bin", NULL);

	SetChild(pShovelModel->m_pModelRootObject, true);
}

Shovel::~Shovel()
{
}

void Shovel::GenerateSwingBoundingBox(XMFLOAT3 playerPos, XMFLOAT3 playerLook)
{
	m_bIsSwingActive = true;

	// 바운딩 박스의 중심: 플레이어 위치에서 전방(Look) 방향으로 1.0f 이동
	XMFLOAT3 forwardOffset = Vector3::ScalarProduct(Vector3::Normalize(playerLook), 1.0f);
	m_attackBoundingBox.Center = Vector3::Add(playerPos, forwardOffset);

	// 바운딩 박스의 크기: (0.5f, 0.5f, 1.0f)
	m_attackBoundingBox.Extents = XMFLOAT3(0.5f, 0.5f, 1.0f);

	UpdateSwingBoundingBox();
}

void Shovel::UpdateSwingBoundingBox()
{
	if (m_bIsSwingActive)
	{

	}
}

void Shovel::DeleteSwingBoundingBox()
{
	m_bIsSwingActive = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Handmap::Handmap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pHandmapModel = pModel;
	if (!pHandmapModel) pHandmapModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Flashlight.bin", NULL);

	SetChild(pHandmapModel->m_pModelRootObject, true);
}

Handmap::~Handmap()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
FlashLight::FlashLight(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pFlashlightModel = pModel;
	if (!pFlashlightModel) pFlashlightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Flashlight.bin", NULL);

	SetChild(pFlashlightModel->m_pModelRootObject, true);
}

FlashLight::~FlashLight()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Whistle::Whistle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pWhistleModel = pModel;
	if (!pWhistleModel) pWhistleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Whistle.bin", NULL);

	SetChild(pWhistleModel->m_pModelRootObject, true);
}

Whistle::~Whistle()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Canister01::Canister01(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pCanister01Model = pModel;
	if (!pCanister01Model) pCanister01Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Canisters_01.bin", NULL);

	SetChild(pCanister01Model->m_pModelRootObject, true);
}

Canister01::~Canister01()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Canister02::Canister02(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pCanister02Model = pModel;
	if (!pCanister02Model) pCanister02Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Canisters_02.bin", NULL);

	SetChild(pCanister02Model->m_pModelRootObject, true);
}

Canister02::~Canister02()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Canister03::Canister03(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pCanister03Model = pModel;
	if (!pCanister03Model) pCanister03Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Canisters_03.bin", NULL);

	SetChild(pCanister03Model->m_pModelRootObject, true);
}

Canister03::~Canister03()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Goldbar::Goldbar(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pGoldbarModel = pModel;
	if (!pGoldbarModel) pGoldbarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Goldbar.bin", NULL);

	SetChild(pGoldbarModel->m_pModelRootObject, true);
}

Goldbar::~Goldbar()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Goldcoin::Goldcoin(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pGoldcoinModel = pModel;
	if (!pGoldcoinModel) pGoldcoinModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Item/Coin.bin", NULL);

	SetChild(pGoldcoinModel->m_pModelRootObject, true);
}

Goldcoin::~Goldcoin()
{
}
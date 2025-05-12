#pragma once

#include "QuadTree.h"

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree;
    std::vector<CGameObject*> m_collisions;
    int frameCounter = 0;

public:
    CCollisionManager() { m_pQuadTree = new CQuadTree(); }
    ~CCollisionManager() { delete m_pQuadTree; }

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth)
    {
        m_pQuadTree->Build(worldBounds, maxObjectsPerNode, maxDepth);
    }

    void InsertObject(CGameObject* object)
    {
        m_pQuadTree->Insert(object);
    }

    void PrintTree()
    {
        m_pQuadTree->PrintTree();
    }

    void Update(CPlayer* player)
    {
        // �÷��̾ ���� ��� Ž��
        QuadTreeNode* playerNode = m_pQuadTree->FindNode(m_pQuadTree->root, player->GetBoundingBox());
        if (!playerNode) return;

        if (frameCounter % 60 == 0) // 60 �����Ӹ��� ���
            cout << playerNode->bounds.Center.x << ", " << playerNode->bounds.Center.z << endl;

        // ��ó ������Ʈ ����
        m_collisions.clear();
        CollectNearbyObjects(playerNode, player->GetBoundingBox(), m_collisions);

        // �浹 �˻� �� ó��
        for (CGameObject* obj : m_collisions)
        {
            if (obj != player && player->GetBoundingBox().Intersects(obj->GetBoundingBox()))
            {
                HandleCollision(player, obj);
            }
        }
    }

    bool IsColliding(const BoundingBox& box1, const BoundingBox& box2)
    {
        // X�� �浹 �˻�
        if (fabs(box1.Center.x - box2.Center.x) > (box1.Extents.x + box2.Extents.x))
            return false;

        // Z�� �浹 �˻�
        if (fabs(box1.Center.z - box2.Center.z) > (box1.Extents.z + box2.Extents.z))
            return false;

        return true;
    }

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb, std::vector<CGameObject*>& collisions)
    {
        if (!node) return;
        for (CGameObject* obj : node->objects)
        {
            if (obj)
            {
                collisions.push_back(obj);
            }
        }
    }

    void HandleCollision(CPlayer* player, CGameObject* obj)
    {
        std::string ObjectFrameName = obj->GetFrameName();

        //if (frameCounter % 60 == 0)
        //    cout << "ObjectFrameName: " << ObjectFrameName << endl;

        if (std::string::npos != ObjectFrameName.find("Map_wall_window")
            || std::string::npos != ObjectFrameName.find("Map_wall_plain")
            || std::string::npos != ObjectFrameName.find("Map_wall_baydoor")
            )
        {
            // �÷��̾�� ���� ��� ���� ��������
            BoundingBox playerBox = player->GetBoundingBox();
            BoundingBox wallBox = obj->GetBoundingBox();

            // �÷��̾�� ���� �߽� �� ����
            XMFLOAT3 playerCenter = playerBox.Center;
            XMFLOAT3 wallCenter = wallBox.Center;
            XMFLOAT3 diff(playerCenter.x - wallCenter.x, playerCenter.y - wallCenter.y, playerCenter.z - wallCenter.z);

            //// �÷��̾� BoundingBox�� 4�� ������ ���
            XMFLOAT3 playerExtents = playerBox.Extents;
            XMFLOAT3 playerVertices[4] = {
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y, playerCenter.z - playerExtents.z), // ����
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y, playerCenter.z - playerExtents.z), // �»��
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y, playerCenter.z + playerExtents.z), // ���ϴ�
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y, playerCenter.z + playerExtents.z), // ���ϴ�
            };

            // �� BoundingBox�� 4�� ������ ���
            XMFLOAT3 wallExtents = wallBox.Extents;
            XMFLOAT3 wallVertices[4] = {
                XMFLOAT3(wallCenter.x - wallExtents.x, playerCenter.y, wallCenter.z - wallExtents.z), // ����
                XMFLOAT3(wallCenter.x + wallExtents.x, playerCenter.y, wallCenter.z - wallExtents.z), // �»��
                XMFLOAT3(wallCenter.x + wallExtents.x, playerCenter.y, wallCenter.z + wallExtents.z), // ���ϴ�
                XMFLOAT3(wallCenter.x - wallExtents.x, playerCenter.y, wallCenter.z + wallExtents.z), // ���ϴ�
            };

            // ���� �о ���� �� �Ÿ� ã��
            XMFLOAT3 pushDirection(0, 0, 0);
            float pushDistance{ 0.0f };
            float pushMargin{ 0.0f };
            float maxExtent = std::max(wallExtents.x, wallExtents.z);
            if (maxExtent == wallExtents.x)
            {
                if (diff.z < 0)
                {
                    // ���� �о�� ��
                    pushDirection = XMFLOAT3(0, 0, -1);
                    pushDistance = playerVertices[2].z - wallVertices[1].z + pushMargin;
                }
                else
                {
                    // �Ʒ��� �о�� ��
                    pushDirection = XMFLOAT3(0, 0, 1);
                    pushDistance = wallVertices[2].z - playerVertices[1].z + pushMargin;
                }
            }
            else
            {
                if (diff.x < 0)
                {
                    // ���������� �о�� ��
                    pushDirection = XMFLOAT3(-1, 0, 0);
                    pushDistance = playerVertices[1].x - wallVertices[0].x + pushMargin;
                }
                else
                {
                    // �������� �о�� ��
                    pushDirection = XMFLOAT3(1, 0, 0);
                    pushDistance = wallVertices[1].x - playerVertices[0].x + pushMargin;
                }
            }

            XMFLOAT3 pushVector(
                pushDirection.x * pushDistance,
                0,
                pushDirection.z * pushDistance
            );

            XMFLOAT3 shift = pushVector;
            player->SetVelocity(XMFLOAT3(0, 0, 0));
            player->Move(shift, false);
            player->CalculateBoundingBox();
            playerBox = player->GetBoundingBox();
        }

        if (std::string::npos != ObjectFrameName.find("Map_barrel")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_01")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_02")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_03")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_04")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_05")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_06")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_07")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_08")
            || std::string::npos != ObjectFrameName.find("Map_pallet_variation_09")
            || std::string::npos != ObjectFrameName.find("Map_pillar")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_01")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_02")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_03")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_04")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_05")
            || std::string::npos != ObjectFrameName.find("Map_shelf_variation_06")
            || std::string::npos != ObjectFrameName.find("Map_shelves_empty")
            || std::string::npos != ObjectFrameName.find("Map_crate_long")
            || std::string::npos != ObjectFrameName.find("Map_garbage_bin")
            || std::string::npos != ObjectFrameName.find("Map_duct_vent")
            || std::string::npos != ObjectFrameName.find("Map_duct_elbow_01")
            || std::string::npos != ObjectFrameName.find("Map_duct_elbow_02")
            || std::string::npos != ObjectFrameName.find("Map_duct_tee")
            || std::string::npos != ObjectFrameName.find("Map_crate_long")
            || std::string::npos != ObjectFrameName.find("Map_crate_short")
            )
        {
            DirectX::XMFLOAT3 playerPos = player->GetPosition();
            BoundingBox playerBox = player->GetBoundingBox();
            BoundingBox objBox = obj->GetBoundingBox();

            // Min, Max ���
            DirectX::XMFLOAT3 playerMin, playerMax, objMin, objMax;
            playerMin.x = playerBox.Center.x - playerBox.Extents.x;
            playerMin.y = playerBox.Center.y - playerBox.Extents.y;
            playerMin.z = playerBox.Center.z - playerBox.Extents.z;
            playerMax.x = playerBox.Center.x + playerBox.Extents.x;
            playerMax.y = playerBox.Center.y + playerBox.Extents.y;
            playerMax.z = playerBox.Center.z + playerBox.Extents.z;

            objMin.x = objBox.Center.x - objBox.Extents.x;
            objMin.y = objBox.Center.y - objBox.Extents.y;
            objMin.z = objBox.Center.z - objBox.Extents.z;
            objMax.x = objBox.Center.x + objBox.Extents.x;
            objMax.y = objBox.Center.y + objBox.Extents.y;
            objMax.z = objBox.Center.z + objBox.Extents.z;

            // ��ħ ũ�� ��� (x, z��)
            DirectX::XMFLOAT3 overlap;
            overlap.x = std::min(playerMax.x, objMax.x) - std::max(playerMin.x, objMin.x);
            overlap.z = std::min(playerMax.z, objMax.z) - std::max(playerMin.z, objMin.z);

            // ��ħ�� ���� ���� �������� �÷��̾� ��ġ ����
            if (overlap.x < overlap.z)
            {
                if (playerPos.x < objBox.Center.x)
                    playerPos.x = objMin.x - playerBox.Extents.x; // �������� �о
                else
                    playerPos.x = objMax.x + playerBox.Extents.x; // ���������� �о
            }
            else
            {
                if (playerPos.z < objBox.Center.z)
                    playerPos.z = objMin.z - playerBox.Extents.z; // �Ʒ��� �о
                else
                    playerPos.z = objMax.z + playerBox.Extents.z; // ���� �о
            }
            player->SetPosition(playerPos); // �÷��̾� ��ġ ����
            player->SetVelocity({ 0.0f, 0.0f, 0.0f }); // �ӵ� ����
            player->CalculateBoundingBox();
            playerBox = player->GetBoundingBox();
        }

        if (std::string::npos != ObjectFrameName.find("Map_corridor_4way")
            || std::string::npos != ObjectFrameName.find("Map_corridor_corner")
            || std::string::npos != ObjectFrameName.find("Map_wall_corridor")
            || std::string::npos != ObjectFrameName.find("Map_corridor_deadend_window")
            || std::string::npos != ObjectFrameName.find("Map_corridor_passthrough")
            || std::string::npos != ObjectFrameName.find("Map_corridor_tee")
            )
        {
            //cout << obj->GetChild()->GetSibling()->GetBoundingBox().Center.x << ", " <<  
            //    obj->GetChild()->GetSibling()->GetBoundingBox().Center.z << ", " <<
            //    obj->GetChild()->GetSibling()->GetBoundingBox().Extents.x << ", " <<
            //    obj->GetChild()->GetSibling()->GetBoundingBox().Extents.z << endl;
        }

    }
};


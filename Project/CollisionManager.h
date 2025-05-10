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

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode)
    {   
        m_pQuadTree->Build(worldBounds, maxObjectsPerNode);
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
            cout << playerNode->bounds.Center.x << endl;

        // ��ó ������Ʈ ����
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

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb, std::vector<CGameObject*>& result)
    {
        if (!node || !node->bounds.Intersects(aabb))
            return;

        if (node->isLeaf)
        {
            result.insert(result.end(), node->objects.begin(), node->objects.end());
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                CollectNearbyObjects(node->children[i], aabb, result);
            }
        }
    }

    void HandleCollision(CPlayer* player, CGameObject* b)
    {
        if (!b || !b->GetChild())
            return;

        std::string nameB = b->GetChild()->GetFrameName();
        std::string nameB2 = b->GetFrameName();

        if (std::string::npos != nameB.find("wall") || std::string::npos != nameB2.find("wall"))
        {
            // �÷��̾�� ���� ��� ���� ��������
            BoundingBox playerBox = player->GetBoundingBox();
            BoundingBox wallBox = b->GetBoundingBox();

            auto collisionPair = std::make_pair(player, b);
            static std::set<std::pair<CGameObject*, CGameObject*>> handledCollisions;
            if (handledCollisions.find(collisionPair) != handledCollisions.end())
                return;
            handledCollisions.insert(collisionPair);

            // �÷��̾�� ���� �߽� �� ����
            XMFLOAT3 playerCenter = playerBox.Center;
            XMFLOAT3 wallCenter = wallBox.Center;
            XMFLOAT3 diff(playerCenter.x - wallCenter.x, playerCenter.y - wallCenter.y, playerCenter.z - wallCenter.z);

            // ���� Extents���� ���� ū �� ã�� (���� �о ����)
            XMFLOAT3 wallExtents = wallBox.Extents;
            XMFLOAT3 pushDirection(0, 0, 0);

            float maxExtent = wallExtents.x;
            if (wallExtents.z > maxExtent) maxExtent = wallExtents.z;

            // z�� ��� x ��������, x�� ��� z�������� ������
            if (maxExtent == wallExtents.x)
            {
                cout << "x";
                //player->Move(XMFLOAT3(0, 0, 1), false);
                pushDirection = XMFLOAT3(diff.x > 0 ? -0.1f : 0.1f, 0, 0);
            }
            else
            {
                cout << "z";
                //player->Move(XMFLOAT3(1, 0, 0), false);
                pushDirection = XMFLOAT3(0, 0, diff.z > 0 ? -0.1f : 0.1f);
            }
            
            //// �÷��̾� BoundingBox�� 8�� ������ ���
            XMFLOAT3 playerExtents = playerBox.Extents;
            XMFLOAT3 vertices[8] = {
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y - playerExtents.y, playerCenter.z - playerExtents.z),
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y - playerExtents.y, playerCenter.z - playerExtents.z),
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y + playerExtents.y, playerCenter.z - playerExtents.z),
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y + playerExtents.y, playerCenter.z - playerExtents.z),
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y - playerExtents.y, playerCenter.z + playerExtents.z),
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y - playerExtents.y, playerCenter.z + playerExtents.z),
                XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y + playerExtents.y, playerCenter.z + playerExtents.z),
                XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y + playerExtents.y, playerCenter.z + playerExtents.z)
            };

            // ���� ���
            XMFLOAT3 wallMin(wallCenter.x - wallExtents.x, wallCenter.y - wallExtents.y, wallCenter.z - wallExtents.z);
            XMFLOAT3 wallMax(wallCenter.x + wallExtents.x, wallCenter.y + wallExtents.y, wallCenter.z + wallExtents.z);

            // �о�� �Ÿ� ��� (�浹 ���� + ������)
            XMFLOAT3 penetrationDepth(
                (playerBox.Extents.x + wallBox.Extents.x) - std::abs(diff.x),
                (playerBox.Extents.y + wallBox.Extents.y) - std::abs(diff.y),
                (playerBox.Extents.z + wallBox.Extents.z) - std::abs(diff.z)
            );

            float pushDistance = 0.0f;
            if (maxExtent == wallExtents.x)
                pushDistance = penetrationDepth.x + 0.001f;
            else
                pushDistance = penetrationDepth.z + 0.001f;

            // �о�� ����
            XMFLOAT3 pushVector(
                pushDirection.x * pushDistance,
                0,
                pushDirection.z * pushDistance
            );

            // ���� ��ġ���� ��� �̵� ���ͷ� ��ȯ
            XMFLOAT3 currentPosition(player->m_xmf4x4World._41, player->m_xmf4x4World._42, player->m_xmf4x4World._43);
            XMFLOAT3 shift = pushVector;

            player->Move(shift, false);
        }


    }
};


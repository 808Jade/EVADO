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

        std::string ObjectFrameName = b->GetFrameName();

        if (frameCounter % 60 == 0)
            cout << "ObjectFrameName: " << ObjectFrameName << endl;

        if (std::string::npos != ObjectFrameName.find("Map_wall_window") || std::string::npos != ObjectFrameName.find("Map_wall_plain"))
        {
            // �÷��̾�� ���� ��� ���� ��������
            BoundingBox playerBox = player->GetBoundingBox();
            BoundingBox wallBox = b->GetBoundingBox();

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
                    cout << "���� �о�� ��" << endl;
                    pushDirection = XMFLOAT3(0, 0, -1);
                    pushDistance = playerVertices[2].z - wallVertices[1].z + pushMargin;
                }
                else
                {
                    // �Ʒ��� �о�� ��
                    cout << "�Ʒ��� �о�� ��" << endl;
                    pushDirection = XMFLOAT3(0, 0, 1);
                    pushDistance = wallVertices[2].z - playerVertices[1].z + pushMargin;
                }
            }
            else
            {
                if (diff.x < 0)
                {
                    // ���������� �о�� ��
                    cout << "���������� �о�� ��" << endl;
                    pushDirection = XMFLOAT3(-1, 0, 0);
                    pushDistance = playerVertices[1].x - wallVertices[0].x + pushMargin;
                }
                else
                {
                    // �������� �о�� ��
                    cout << "�������� �о�� ��" << endl;
                    pushDirection = XMFLOAT3(1, 0, 0);
                    pushDistance = wallVertices[1].x - playerVertices[0].x + pushMargin;
                }
            }

            // �о�� ����
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
    }
};


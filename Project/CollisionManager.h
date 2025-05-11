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
        //if (frameCounter % 60 == 0) // 60 �����Ӹ��� ���
        //    cout << playerNode->bounds.Center.x << endl;

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

        if (frameCounter % 60 == 0)
            cout << "Child(): " << nameB << "Self: " << nameB2 << endl;

        if (std::string::npos != nameB.find("wall") || std::string::npos != nameB2.find("wall"))
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
                    pushDirection = XMFLOAT3(0, 0, 1);
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
                    pushDirection = XMFLOAT3(-1, 0, 0);
                    pushDistance = wallVertices[0].x - playerVertices[1].x + pushMargin;
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

// ���� �÷��̾�� ���� �߽� �� ũ��
            //XMFLOAT3 playerCenter = playerBox.Center;
            //XMFLOAT3 playerExtents = playerBox.Extents;
            //XMFLOAT3 wallCenter = wallBox.Center;
            //XMFLOAT3 wallExtents = wallBox.Extents;
            //XMFLOAT3 wallMin(wallCenter.x - wallExtents.x, wallCenter.y - wallExtents.y, wallCenter.z - wallExtents.z);
            //XMFLOAT3 wallMax(wallCenter.x + wallExtents.x, wallCenter.y + wallExtents.y, wallCenter.z + wallExtents.z);

            //XMFLOAT3 adjustedCenter = playerCenter;

            //float margin = 0.5;

            //// x�� ����: �÷��̾� Center�� �� ������ ������ �� �ܰ����� ����
            //if (playerCenter.x < wallMin.x)
            //    adjustedCenter.x = wallMin.x + margin; // ���� �ܰ����� �̵�
            //else if (playerCenter.x > wallMax.x)
            //    adjustedCenter.x = wallMax.x - margin; // ������ �ܰ����� �̵�
            //// z�� ����: �÷��̾� Center�� �� ������ ������ �� �ܰ����� ����
            //else if (playerCenter.z < wallMin.z)
            //    adjustedCenter.z = wallMin.z + margin; // ���� �ܰ����� �̵�
            //else if (playerCenter.z > wallMax.z)
            //    adjustedCenter.z = wallMax.z - margin; // ���� �ܰ����� �̵�
            //else
            //    return;

            //// y�� ���� (�ʿ� �� ����)
            //adjustedCenter.y = playerCenter.y;

            //if (adjustedCenter.x != playerCenter.x || adjustedCenter.z != playerCenter.z)
            //{
            //    player->SetPosition(adjustedCenter);
            //    player->CalculateBoundingBox();

            //    playerBox = player->GetBoundingBox();
            //}
        }
    }
};


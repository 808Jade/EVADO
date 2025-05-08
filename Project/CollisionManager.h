#pragma once

#include "Object.h"
#include "QuadTree.h"

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree;

public:
    CCollisionManager() { m_pQuadTree = new CQuadTree(); }
    ~CCollisionManager() { delete m_pQuadTree; }

    void Build(const BoundingBox& worldBounds, int maxDepth, int maxObjectsPerNode)
    {   
        m_pQuadTree->Build(worldBounds, maxDepth, maxObjectsPerNode);
    }

    void InsertObject(CGameObject* object) { m_pQuadTree->Insert(object); }

    void Update(CGameObject* player, std::vector<CGameObject*>& collisions)
    {
        // �÷��̾ ���� ��� Ž�� (FindNode �޼��� �߰� �ʿ�)
        QuadTreeNode* playerNode = m_pQuadTree->FindNode(m_pQuadTree->root, player->GetBoundingBox());
        if (!playerNode) return;

        // ��ó ������Ʈ ����
        CollectNearbyObjects(playerNode, player->GetBoundingBox(), collisions);

        // �浹 �˻� �� ó��
        for (CGameObject* obj : collisions)
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

    void HandleCollision(CGameObject* a, CGameObject* b)
    {
        // �浹 ó�� ���� (��: ��ġ ����, �̺�Ʈ �߻� ��)
        OutputDebugStringA("Collision detected between objects\n");
        // ��: a->SetPosition(...), b->SetPosition(...)
    }
};


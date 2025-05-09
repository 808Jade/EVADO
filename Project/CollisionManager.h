#pragma once

#include "QuadTree.h"

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree;
    std::vector<CGameObject*> m_collisions;

public:
    CCollisionManager() { m_pQuadTree = new CQuadTree(); }
    ~CCollisionManager() { delete m_pQuadTree; }

    void Build(const BoundingBox& worldBounds, int maxDepth, int maxObjectsPerNode)
    {   
        m_pQuadTree->Build(worldBounds, maxDepth, maxObjectsPerNode);
    }

    void InsertObject(CGameObject* object) { m_pQuadTree->Insert(object); }

    void Update(CGameObject* player)
    {
        // �÷��̾ ���� ��� Ž��
        QuadTreeNode* playerNode = m_pQuadTree->FindNode(m_pQuadTree->root, player->GetBoundingBox());
        if (!playerNode) return;

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

    void HandleCollision(CGameObject* a, CGameObject* b)
    {
        // �浹 ó�� ���� (��: ��ġ ����, �̺�Ʈ �߻� ��)
       cout << "Collision detected between objects" << endl;
        // ��: a->SetPosition(...), b->SetPosition(...)
    }
};


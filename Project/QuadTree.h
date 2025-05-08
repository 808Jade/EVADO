#pragma once

#include "Object.h"

struct QuadTreeNode
{
    BoundingBox                 bounds;         // ����� ���
    std::vector<CGameObject*>   objects;        // �� ��忡 ���� ������Ʈ
    QuadTreeNode*               children[4];    // �ڽ� ��� (4��)
    bool                        isLeaf;         // ���� ��� ����

    QuadTreeNode(const BoundingBox& bounds) : bounds(bounds), isLeaf(true)
    {
        for (int i = 0; i < 4; i++)
            children[i] = nullptr;
    }
};

class CQuadTree
{
public:
    QuadTreeNode*   root;
    int             maxDepth;
    int             maxObjectsPerNode;

    CQuadTree() : root(nullptr), maxDepth(0), maxObjectsPerNode(0) {}
    ~CQuadTree()
    {
        DeleteNode(root);
    }

    void Build(const BoundingBox& worldBounds, int depth, int maxObjects)
    {
        maxDepth = depth;
        maxObjectsPerNode = maxObjects;
        root = new QuadTreeNode(worldBounds);
        Subdivide(root, 0);
    }

    void Insert(CGameObject* object)
    {
        InsertObject(root, object);
    }

private:
    void Subdivide(QuadTreeNode* node, int depth)
    {
        if (depth >= maxDepth) return;

        XMFLOAT3 center = node->bounds.Center;
        XMFLOAT3 extents = node->bounds.Extents;
        float halfX = extents.x * 0.5f;
        float halfY = extents.y * 0.5f;

        BoundingBox childBounds[4];
        childBounds[0] = BoundingBox(XMFLOAT3(center.x - halfX, center.y - halfY, center.z), XMFLOAT3(halfX, halfY, extents.z)); // ���� �Ʒ�
        childBounds[1] = BoundingBox(XMFLOAT3(center.x + halfX, center.y - halfY, center.z), XMFLOAT3(halfX, halfY, extents.z)); // ������ �Ʒ�
        childBounds[2] = BoundingBox(XMFLOAT3(center.x - halfX, center.y + halfY, center.z), XMFLOAT3(halfX, halfY, extents.z)); // ���� ��
        childBounds[3] = BoundingBox(XMFLOAT3(center.x + halfX, center.y + halfY, center.z), XMFLOAT3(halfX, halfY, extents.z)); // ������ ��

        for (int i = 0; i < 4; i++)
        {
            node->children[i] = new QuadTreeNode(childBounds[i]);
            Subdivide(node->children[i], depth + 1);
        }
        node->isLeaf = false;
    }

    void InsertObject(QuadTreeNode* node, CGameObject* object, int depth)
    {
        if (node->isLeaf)
        {
            if (node->objects.size() < maxObjectsPerNode || depth >= maxDepth)
            {
                node->objects.push_back(object);
                object->m_pNode = node; // CGameObject�� node ������ ����
            }
            else
            {
                Subdivide(node, depth + 1);
                node->objects.push_back(object); // �ӽ� ����
                for (CGameObject* obj : node->objects)
                    Redistribute(node, obj);
                node->objects.clear();
            }
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                if (node->children[i]->bounds.Intersects(object->GetBoundingBox())) // CGameObject���� BoundingBox ��ȯ ����
                    InsertObject(node->children[i], object, maxDepth);
            }
        }
    }

    void Redistribute(QuadTreeNode* node, CGameObject* object)
    {
        for (int i = 0; i < 4; i++)
        {
            if (node->children[i]->bounds.Intersects(object->GetBoundingBox()))
            {
                InsertObject(node->children[i], object, maxDepth); // depth�� maxDepth�� ����
                break;
            }
        }
    }

    void DeleteNode(QuadTreeNode* node)
    {
        if (!node) return;
        for (int i = 0; i < 4; i++)
            DeleteNode(node->children[i]);
        delete node;
    }
};


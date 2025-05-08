#pragma once

struct QuadTreeNode
{
    BoundingBox                 bounds;         // ����� ���
    std::vector<CGameObject*>   objects;        // �� ��忡 ���� ������Ʈ
    QuadTreeNode*               children[4];    // �ڽ� ��� (4��)
    bool                        isLeaf;         // ���� ��� ����
};

class CQuadTree
{
public:
    QuadTreeNode* root;
    int maxDepth;
    int maxObjectsPerNode;

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

        //float halfWidth = node->bounds.width * 0.5f;
        //float halfHeight = node->bounds.height * 0.5f;
        //float x = node->bounds.minX;
        //float y = node->bounds.minY;

        //node->children[0] = new QuadTreeNode(BoundingBox(x, y, halfWidth, halfHeight));
        //node->children[1] = new QuadTreeNode(BoundingBox(x + halfWidth, y, halfWidth, halfHeight));
        //node->children[2] = new QuadTreeNode(BoundingBox(x, y + halfHeight, halfWidth, halfHeight));
        //node->children[3] = new QuadTreeNode(BoundingBox(x + halfWidth, y + halfHeight, halfWidth, halfHeight));

        node->isLeaf = false;
        for (int i = 0; i < 4; i++)
            Subdivide(node->children[i], depth + 1);
    }

    void InsertObject(QuadTreeNode* node, CGameObject* object)
    {
        if (node->isLeaf)
        {
            //if (node->objects.size() < maxObjectsPerNode || depth >= maxDepth)
            //{
            //    node->objects.push_back(object);
            //    object->node = node;
            //}
            //else
            //{
            //    Subdivide(node, depth + 1);
            //    node->objects.push_back(object); // �ӽ� ����
            //    for (CGameObject* obj : node->objects)
            //        Redistribute(node, obj);
            //    node->objects.clear();
            //}
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                //if (node->children[i]->bounds.Intersects(object->boundingBox))
                //    InsertObject(node->children[i], object);
            }
        }
    }
};


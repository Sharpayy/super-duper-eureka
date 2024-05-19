#include <iostream>
#include <vector>

class PointQT {
public:
    PointQT() = default;
    PointQT(float x, float y) {
        this->x = x;
        this->y = y;
    }

    PointQT(int x, int y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const PointQT& other) const {
        return (x == other.x && y == other.y);
    }

    float x;
    float y;
};

class RectQT {
public:
    RectQT() = default;

    RectQT(const PointQT point, float width, float height) {
        this->point = point;
        this->width = width;
        this->height = height;
    }

    RectQT(const PointQT point, int width, int height) {
        this->point = point;
        this->width = width;
        this->height = height;
    }

    bool operator==(const RectQT& other) const {
        return (point == other.point && width == other.width && height == other.height);
    }

    bool intersect(const RectQT other) const {
        return (other.point.x < (point.x + width) && (other.point.x + other.width) > point.x &&
            other.point.y < (point.y + height) && (other.point.y + other.height) > point.y);
    }

    PointQT point;
    float width;
    float height;
};

#define TOP_LEFT 1
#define TOP_RIGHT 2
#define BOTTOM_LEFT 3
#define BOTTOM_RIGHT 4

template <typename T>
class QT {
public:
    QT() {
        root = new Node{};
        _alloc(1);
    }
    QT(int w, int h)  {
        this->w = w;
        this->h = h;
        root = new Node{};
        _alloc(1);
    }
    ~QT() {
        std::vector<Node*> vdata;
        _getNodesRec(root, vdata);
        for (Node* n : vdata) {
            delete n->p;
            delete n;
        }
    }

    void _push(T* data, PointQT p) {
        Node* n = root;
        RectQT rect = { PointQT{-w / 2.0f, -h / 2.0f}, w / 2.0f, h / 2.0f };
        uint8_t dir;
        while (n->data || n == root) {
            dir = getDirection(p, rect);
            _calcDim(rect, dir, n);
        }
        n->_init(data, new PointQT{ p.x, p.y });
    }

    uint8_t getDirection(const PointQT& p, const RectQT& rect) {
        float left = rect.point.x;
        float right = rect.point.x + rect.width;
        float top = rect.point.y;
        float bottom = rect.point.y + rect.height;

        if (p.x >= left && p.x <= right) {
            if (p.y >= top && p.y <= bottom) {
                return TOP_LEFT;
            }
            else if (p.y > bottom && p.y <= bottom + rect.height) {
                return BOTTOM_LEFT;
            }
        }
        else if (p.x > right && p.x <= right + rect.width) {
            if (p.y >= top && p.y <= bottom) {
                return TOP_RIGHT;
            }
            else if (p.y > bottom && p.y <= bottom + rect.height) {
                return BOTTOM_RIGHT;
            }
        }
        return 0;
    }

    bool _collide(T* data, float w, float h) {
        Node* n = nullptr;
        Node* base = nullptr;
        _finNodeRec(root, data, n, base);
        if (n) {
            RectQT ob1;
            PointQT* p = n->p;
            ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
            return _collideRec(base, ob1, n);
        }
        return false;
    }

    bool _collidePoint(PointQT& p, float w, float h) {
        RectQT ob1;
        ob1 = RectQT{ PointQT{p.x - w / 2.0f, p.y - h / 2.0f }, w, h };
        return _collideRec(root, ob1, root);
    }

    void _alloc(int depth) {
        _allocRec(root, 0, depth);
    }

    void _clear() {
        _removeDataRec(root);
    }

    std::vector<T*> _rebuild(T* data) {
        Node* n = nullptr;
        Node* b = nullptr;
        _finNodeRec(root, data, n, b);
        return _rebuildRec(n);
    }

private:
    class Node {
    public:
        Node() = default;
        void _init(T* data = nullptr, PointQT* p = nullptr) {
            for (int i = 0; i < 4; i++) {
                if(nodes[i] == NULL) nodes[i] = new Node{};
            }
            this->p = p;
            this->data = data;
        }
        ~Node() {
            delete this->p;
            this->p = nullptr;
            this->data = nullptr;
        }

        PointQT* p;
        T* data;
        Node* nodes[4];
    };

    void _calcDim(RectQT& rect, uint8_t& dir, Node*& n) {
        switch (dir) {
        case TOP_LEFT:
            n = n->nodes[0];
            break;
        case TOP_RIGHT:
            n = n->nodes[1];
            rect.point.x += rect.width;
            break;
        case BOTTOM_LEFT:
            n = n->nodes[2];
            rect.point.y += rect.height;
            break;
        case BOTTOM_RIGHT:
            n = n->nodes[3];
            rect.point.x += rect.width;
            rect.point.y += rect.height;
            break;
        default:
            break;
        }
        rect.width /= 2.0f;
        rect.height /= 2.0f;
    }

    void _finNodeRec(Node* n, T* data, Node*& finallNode, Node*& base) {
        if (n) {
            if (!base) base = n;
            if (data == n->data) finallNode = n;
            for (int i = 0; i < 4; i++) {
                _finNodeRec(n->nodes[i], data, finallNode, base);
            }
        }
    }

    std::vector<T*> _rebuildRec(Node* n) {
        std::vector<T*> dataToRebuild;
        Node* b = nullptr;
        _getDataRec(n, dataToRebuild);
        _removeDataRec(n);
        return dataToRebuild;
    }

    void _getDataRec(Node* n, std::vector<T*>& data) {
        if (n->data) {
            data.push_back(n->data);
            for (int i = 0; i < 4; i++) {
                _getDataRec(n->nodes[i], data);
            }
        }
    }

    void _getNodesRec(Node* n, std::vector<Node*>& data) {
        if (n->data || n == root) {
            data.push_back(n);
            for (int i = 0; i < 4; i++) {
                _getNodesRec(n->nodes[i], data);
            }
        }
    }

    void _getNodesIntersecRec(Node* n, std::vector<Node*>& data, RectQT& rect) {
        if (n->data && rect.intersect()) {
            data.push_back(n);
            for (int i = 0; i < 4; i++) {
                _getNodesRec(n->nodes[i], data);
            }
        }
    }

    void _removeDataRec(Node* n) {
        if (n->data || n == root) {
            for (int i = 0; i < 4; i++) {
                _removeDataRec(n->nodes[i]);
            }
            n->~Node();
        }
    }

    bool _collideRec(Node* n, const RectQT& ob1, Node* ignore) {
        if (n->data || n == root) {
            if (n != ignore && n->p) {
                RectQT ob2 = RectQT{ PointQT{n->p->x - ob1.width / 2.0f, n->p->y - ob1.height / 2.0f}, ob1.width, ob1.height };
                if (ob1.intersect(ob2)) {
                    return true;
                }
            }
            for (int i = 0; i < 4; i++) {
                if (_collideRec(n->nodes[i], ob1, ignore)) {
                    return true;
                }
            }
        }
        return false;
    }

    void _allocRec(Node* n, int depth, int depthMax) {
        if (depth != depthMax) {
            depth++;
            for (int i = 0; i < 4; i++) {
                if (n->nodes[i] == nullptr) n->_init();
                _allocRec(n->nodes[i], depth, depthMax);
            }
        }
    }

    Node* root;
    int w, h;
};

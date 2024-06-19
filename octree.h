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

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3

template <typename T>
class QT {
public:
    QT() {
        this->amount = 0;
        root = new Node{};
        this->w = 1;
        this->h = 1;
        _alloc(1);
        //root->_init(nullptr, { w,h });
    }
    QT(int w, int h) {
        this->w = w;
        this->h = h;
        this->amount = 0;
        root = new Node{};
        _alloc(1);
        //root->_init(nullptr, new PointQT{});
    }
    ~QT() {
        std::vector<Node*> vdata;
        _getNodesRec(root, vdata);
        for (Node* n : vdata) {
            delete n->p;
            delete n;
        }
    }

    int _getSize() {
        return this->amount;
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
        amount++;
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
        _findNodeRec(root, data, n, base);
        if (n) {
            RectQT ob1;
            PointQT* p = n->p;
            ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
            return _collideRec(base, ob1, n);
        }
        return false;
    }

    bool _collide(T* data, float w, float h, std::vector<T*>& dataV) {
        Node* n = nullptr;
        Node* base = nullptr;
        _findNodeRec(root, data, n, base);
        if (n) {
            RectQT ob1;
            PointQT* p = n->p;
            ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
            RectQT rootRect = RectQT{ PointQT{-this->w / 2.0f, -this->h / 2.0f }, this->w, this->h };
            _collideRec(base, ob1, rootRect, n, dataV);
            return dataV.size();
        }
        return false;
    }

    bool _collidePoints(PointQT& p, float w, float h, std::vector<T*>& data) {
        RectQT ob1;
        ob1 = RectQT{ PointQT{p.x - w / 2.0f, p.y - h / 2.0f }, w, h };
        RectQT rootRect = RectQT{ PointQT{-this->w / 2.0f, -this->h / 2.0f }, this->w, this->h };
        _collideRec(root, ob1, rootRect, root, data);
        return data.size();
    }

    bool _collidePoint(PointQT& p, float w, float h) {
        RectQT ob1;
        ob1 = RectQT{ PointQT{p.x - w / 2.0f, p.y - h / 2.0f }, w, h };
        RectQT rootRect = RectQT{ PointQT{-this->w / 2.0f, -this->h / 2.0f }, this->w, this->h };
        return _collideRec(root, ob1, rootRect, root);
    }

    void _alloc(int depth) {
        _allocRec(root, 0, depth);
    }

    void _clear() {
        _removeDataRec(root);
        amount = 0;
    }

    std::vector<T*> _rebuild(T* data) {
        Node* n = nullptr;
        Node* b = nullptr;
        _findNodeRec(root, data, n, b);
        return _rebuildRec(n);
    }

private:
    class Node {
    public:
        Node() = default;
        void _init(T* data = nullptr, PointQT* p = nullptr) {
            for (int i = 0; i < 4; i++) {
                if (nodes[i] == NULL) nodes[i] = new Node{};
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

    void _calcDim(RectQT& rect, uint8_t& dir) {
        rect.width /= 2.0f;
        rect.height /= 2.0f;
        switch (dir) {
        case TOP_LEFT:
            break;
        case TOP_RIGHT:
            rect.point.x += rect.width;
            break;
        case BOTTOM_LEFT:
            rect.point.y += rect.height;
            break;
        case BOTTOM_RIGHT:
            rect.point.x += rect.width;
            rect.point.y += rect.height;
            break;
        default:
            break;
        }
    }

    void _findNodeRec(Node* n, T* data, Node*& finallNode, Node*& base) {
        if (finallNode) return;
        else if (n->data || n == root) {
            if (!base) base = n;
            if (data == n->data) {
                finallNode = n;
                return;
            }

            for (int i = 0; i < 4; i++) {
                if (!finallNode) _findNodeRec(n->nodes[i], data, finallNode, base);
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

    void _collideRec(Node* n, const RectQT& ob1, RectQT nodeRect, Node* ignore, std::vector<T*>& data) {
        if (n != ignore && n->p) {
            RectQT ob2 = RectQT{ PointQT{n->p->x - ob1.width / 2.0f, n->p->y - ob1.height / 2.0f}, ob1.width, ob1.height };
            if (ob1.intersect(ob2)) {
                data.push_back(n->data);
            }
        }
        RectQT nRect;
        for (uint8_t i = 0; i < 4; i++) {
            nRect = nodeRect;
            _calcDim(nRect, i);

            if (n->nodes[i]->data) {
                if (nRect.intersect(ob1)) {
                    _collideRec(n->nodes[i], ob1, nRect, ignore, data);
                }
            }
        }
    }

    bool _collideRec(Node* n, const RectQT& ob1, RectQT nodeRect, Node* ignore) {
        if (n->data || n == root) {
            if (n != ignore && n->p) {
                RectQT ob2 = RectQT{ PointQT{n->p->x - ob1.width / 2.0f, n->p->y - ob1.height / 2.0f}, ob1.width, ob1.height };
                if (ob1.intersect(ob2)) {
                    return true;
                }
            }
            RectQT nRect;
            for (uint8_t i = 0; i < 4; i++) {
                nRect = nodeRect;
                _calcDim(nRect, i);

                if (n->nodes[i]->data) {
                    if (nRect.intersect(ob1)) {
                        if (_collideRec(n->nodes[i], ob1, nRect, ignore)) {
                            return true;
                        }
                    }
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

    int amount;
    Node* root;
    int w, h;
};
//#include <iostream>
//#include <vector>
//
//class PointQT {
//public:
//    PointQT() = default;
//    PointQT(float x, float y) {
//        this->x = x;
//        this->y = y;
//    }
//
//    PointQT(int x, int y) {
//        this->x = x;
//        this->y = y;
//    }
//
//    bool operator==(const PointQT& other) const {
//        return (x == other.x && y == other.y);
//    }
//
//    float x;
//    float y;
//};
//
//class RectQT {
//public:
//    RectQT() = default;
//
//    RectQT(const PointQT point, float width, float height) {
//        this->point = point;
//        this->width = width;
//        this->height = height;
//    }
//
//    RectQT(const PointQT point, int width, int height) {
//        this->point = point;
//        this->width = width;
//        this->height = height;
//    }
//
//    bool operator==(const RectQT& other) const {
//        return (point == other.point && width == other.width && height == other.height);
//    }
//
//    bool intersect(const RectQT other) const {
//        return (other.point.x < (point.x + width) && (other.point.x + other.width) > point.x &&
//            other.point.y < (point.y + height) && (other.point.y + other.height) > point.y);
//    }
//
//    PointQT point;
//    float width;
//    float height;
//};
//
//#define TOP_LEFT 1
//#define TOP_RIGHT 2
//#define BOTTOM_LEFT 3
//#define BOTTOM_RIGHT 4
//
//template <typename T>
//class QT {
//public:
//    QT() {
//        root = new Node{};
//        _alloc(1);
//    }
//    QT(uint32_t w, uint32_t h, uint8_t size) {
//        this->w = w;
//        this->h = h;
//        root = new Node{};
//        _alloc(1);
//    }
//    ~QT() {
//        std::vector<Node*> vdata;
//        _getNodesRec(root, vdata);
//        for (Node* n : vdata) {
//            delete n->p;
//            delete n;
//        }
//    }
//
//    void _push(T* data, PointQT p) {
//        Node* n = root;
//        RectQT rect = { PointQT{-w / 2.0f, -h / 2.0f}, w / 2.0f, h / 2.0f };
//        uint8_t dir;
//        while (n->data || n == root) {
//            dir = getDirection(p, rect);
//            _calcDim(rect, dir, n);
//        }
//        n->_init(data, new PointQT{ p.x, p.y });
//    }
//
//    uint8_t getDirection(const PointQT& p, const RectQT& rect) {
//        float left = rect.point.x;
//        float right = rect.point.x + rect.width;
//        float top = rect.point.y;
//        float bottom = rect.point.y + rect.height;
//
//        if (p.x >= left && p.x <= right) {
//            if (p.y >= top && p.y <= bottom) {
//                return TOP_LEFT;
//            }
//            else if (p.y > bottom && p.y <= bottom + rect.height) {
//                return BOTTOM_LEFT;
//            }
//        }
//        else if (p.x > right && p.x <= right + rect.width) {
//            if (p.y >= top && p.y <= bottom) {
//                return TOP_RIGHT;
//            }
//            else if (p.y > bottom && p.y <= bottom + rect.height) {
//                return BOTTOM_RIGHT;
//            }
//        }
//        return 0;
//    }
//
//    bool _collide(T* data, float w, float h) {
//        Node* n = nullptr;
//        Node* base = nullptr;
//        _findNodeRec(root, data, n, base);
//        if (n) {
//            RectQT ob1;
//            PointQT* p = n->p;
//            ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
//            return _collideRec(base, ob1, n);
//        }
//        return false;
//    }
//
//    bool _collide(T* data, float w, float h, T*& c) {
//        Node* n = nullptr;
//        Node* base = nullptr;
//        _findNodeRec(root, data, n, base);
//        if (n) {
//            RectQT ob1;
//            PointQT* p = n->p;
//            ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
//            return _collideRec(base, ob1, n, c);
//        }
//        return false;
//    }
//
//    bool _collidePoint(PointQT& p, float w, float h, T*& data) {
//        RectQT ob1;
//        ob1 = RectQT{ PointQT{p.x - w / 2.0f, p.y - h / 2.0f }, w, h };
//        return _collideRec(root, ob1, root, data);
//    }
//
//    bool _collidePoint(PointQT& p, float w, float h) {
//        RectQT ob1;
//        ob1 = RectQT{ PointQT{p.x - w / 2.0f, p.y - h / 2.0f }, w, h };
//        return _collideRec(root, ob1, root);
//    }
//
//    void _alloc(int depth) {
//        _allocRec(root, 0, depth);
//    }
//
//    void _clear() {
//        _removeDataRec(root);
//    }
//
//    std::vector<T*> _rebuild(T* data) {
//        Node* n = nullptr;
//        Node* b = nullptr;
//        _findNodeRec(root, data, n, b);
//        return _rebuildRec(n);
//    }
//
//private:
//    class Node {
//    public:
//        Node() = default;
//        void _init(T* data = nullptr, PointQT* p = nullptr) {
//            for (int i = 0; i < 4; i++) {
//                if (nodes[i] == NULL) nodes[i] = new Node{};
//            }
//            s = 0;
//            this->p = p;
//            this->data = new T[size];
//            this->data[s++];
//        }
//        ~Node() {
//            delete this->p;
//            this->p = nullptr;
//            this->data = nullptr;
//        }
//
//        PointQT* p;
//        T* data;
//        uint8_t s;
//        Node* nodes[4];
//    };
//
//    void _calcDim(RectQT& rect, uint8_t& dir, Node*& n) {
//        switch (dir) {
//        case TOP_LEFT:
//            n = n->nodes[0];
//            break;
//        case TOP_RIGHT:
//            n = n->nodes[1];
//            rect.point.x += rect.width;
//            break;
//        case BOTTOM_LEFT:
//            n = n->nodes[2];
//            rect.point.y += rect.height;
//            break;
//        case BOTTOM_RIGHT:
//            n = n->nodes[3];
//            rect.point.x += rect.width;
//            rect.point.y += rect.height;
//            break;
//        default:
//            break;
//        }
//        rect.width /= 2.0f;
//        rect.height /= 2.0f;
//    }
//
//    void _findNodeRec(Node* n, T* data, Node*& finallNode, Node*& base) {
//        if (finallNode) return;
//        else if (n->data || n == root) {
//            if (!base) base = n;
//            if (data == n->data) {
//                finallNode = n;
//                return;
//            }
//            for (int i = 0; i < 4; i++) {
//                _findNodeRec(n->nodes[i], data, finallNode, base);
//            }
//        }
//    }
//
//    std::vector<T*> _rebuildRec(Node* n) {
//        std::vector<T*> dataToRebuild;
//        Node* b = nullptr;
//        _getDataRec(n, dataToRebuild);
//        _removeDataRec(n);
//        return dataToRebuild;
//    }
//
//    void _getDataRec(Node* n, std::vector<T*>& data) {
//        if (n->data) {
//            data.push_back(n->data);
//            for (int i = 0; i < 4; i++) {
//                _getDataRec(n->nodes[i], data);
//            }
//        }
//    }
//
//    void _getNodesRec(Node* n, std::vector<Node*>& data) {
//        if (n->data || n == root) {
//            data.push_back(n);
//            for (int i = 0; i < 4; i++) {
//                _getNodesRec(n->nodes[i], data);
//            }
//        }
//    }
//
//    void _getNodesIntersecRec(Node* n, std::vector<Node*>& data, RectQT& rect) {
//        if (n->data && rect.intersect()) {
//            data.push_back(n);
//            for (int i = 0; i < 4; i++) {
//                _getNodesRec(n->nodes[i], data);
//            }
//        }
//    }
//
//    void _removeDataRec(Node* n) {
//        if (n->data || n == root) {
//            for (int i = 0; i < 4; i++) {
//                _removeDataRec(n->nodes[i]);
//            }
//            n->~Node();
//        }
//    }
//
//    bool _collideRec(Node* n, const RectQT& ob1, Node* ignore, T*& data) {
//        if (n->data || n == root) {
//            if (n != ignore && n->p) {
//                RectQT ob2 = RectQT{ PointQT{n->p->x - ob1.width / 2.0f, n->p->y - ob1.height / 2.0f}, ob1.width, ob1.height };
//                if (ob1.intersect(ob2)) {
//                    data = n->data;
//                    return true;
//                }
//            }
//            for (int i = 0; i < 4; i++) {
//                if (_collideRec(n->nodes[i], ob1, ignore, data)) {
//                    return true;
//                }
//            }
//        }
//        return false;
//    }
//
//    bool _collideRec(Node* n, const RectQT& ob1, Node* ignore) {
//        if (n->data || n == root) {
//            if (n != ignore && n->p) {
//                RectQT ob2 = RectQT{ PointQT{n->p->x - ob1.width / 2.0f, n->p->y - ob1.height / 2.0f}, ob1.width, ob1.height };
//                if (ob1.intersect(ob2)) {
//                    return true;
//                }
//            }
//            for (int i = 0; i < 4; i++) {
//                if (_collideRec(n->nodes[i], ob1, ignore)) {
//                    return true;
//                }
//            }
//        }
//        return false;
//    }
//
//    void _allocRec(Node* n, int depth, int depthMax) {
//        if (depth != depthMax) {
//            depth++;
//            for (int i = 0; i < 4; i++) {
//                if (n->nodes[i] == nullptr) n->_init();
//                _allocRec(n->nodes[i], depth, depthMax);
//            }
//        }
//    }
//
//    Node* root;
//    uint8_t w, h;
//    uint8_t size;
//};
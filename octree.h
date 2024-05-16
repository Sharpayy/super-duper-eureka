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

	RectQT(const PointQT point, const float width, const float height) {
		this->point = point;
		this->width = width;
		this->height = height;
	}
	RectQT(const PointQT point, const int width, const int height) {
		this->point = point;
		this->width = (float)width;
		this->height = (float)height;
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
	};
	QT(int w, int h) {
		this->w = w;
		this->h = h;
		root = new Node{};
		_alloc(1);
	}

	void _push(T* data, PointQT p) {
		Node* n = root;
		RectQT rect = { PointQT{0, 0}, w, h };
		uint8_t dir;
		while (n->data || n == root) {
			if (p.x >= rect.point.x && p.x <= rect.point.x + rect.width
				&& p.y >= rect.point.y && p.y <= rect.point.y + rect.height) {
				dir = TOP_LEFT;
			}
			else if (p.x >= rect.point.x + rect.width && p.x <= rect.point.x + 2 * rect.width
				&& p.y >= rect.point.y && p.y <= rect.point.y + rect.height) {
				dir = TOP_RIGHT;
			}
			else if (p.x >= rect.point.x && p.x <= rect.point.x + rect.width
				&& p.y >= rect.point.y + rect.height && p.y <= rect.point.y + 2 * rect.height) {
				dir = BOTTOM_LEFT;
			}
			else if (p.x >= rect.point.x + rect.width && p.x <= rect.point.x + 2 * rect.width
				&& p.y >= rect.point.y + rect.height && p.y <= rect.point.y + 2 * rect.height) {
				dir = BOTTOM_RIGHT;
			}
			rect = _calcDim(rect, dir, &n);
		}
		n->_init(data, new PointQT{ p.x, p.y });


	}

	bool _collide(T* data, float w, float h) {
		Node* n = nullptr;
		Node* base = nullptr;
		_finNodeRec(root, data, n, base);
		if (n) {
			RectQT ob1, ob2;
			PointQT* p = n->p;
			ob1 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h };
			std::vector<Node*> vdata;
			_getNodesRec(base, vdata);
			for (Node* n : vdata) {
				p = n->p;
				ob2 = RectQT{ PointQT{p->x - w / 2.0f, p->y - h / 2.0f }, w, h};
				if (ob1.intersect(ob2) && !(ob1 == ob2)) return true;
			}
		}
		return false;

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
			this->p = p;
			this->data = data;
			for (int i = 0; i < 4; i++) {
				nodes[i] = new Node{};
			}
		}
		//~Node() {
		//	delete nodes[];
		//}

		PointQT* p;
		T* data;
		Node* nodes[4];
	};

	RectQT _calcDim(RectQT rect, uint8_t dir, Node** n) {
		float w, h;
		w = rect.width / 2.0f;
		h = rect.height / 2.0f;
		switch (dir)
		{
		case TOP_LEFT:
			(*n) = (*n)->nodes[0];
			return { PointQT{rect.point.x , rect.point.y }, w, h };
			break;
		case TOP_RIGHT:
			(*n) = (*n)->nodes[1];
			return { PointQT{rect.point.x + w, rect.point.y }, w, h };
			break;
		case BOTTOM_LEFT:
			(*n) = (*n)->nodes[2];
			return { PointQT{rect.point.x, rect.point.y + h }, w, h };
			break;
		case BOTTOM_RIGHT:
			(*n) = (*n)->nodes[3];
			return { PointQT{rect.point.x + w, rect.point.y + h }, w, h };
			break;
		default:
			break;
		}


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
		if (n->data) {
			data.push_back(n);
			for (int i = 0; i < 4; i++) {
				_getNodesRec(n->nodes[i], data);
			}
		}
	}

	void _removeDataRec(Node* n) {
		if (n) {
			n->data = nullptr;
			n->p = nullptr;
			for (int i = 0; i < 4; i++) {
				_removeDataRec(n->nodes[i]);
			}
		}
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
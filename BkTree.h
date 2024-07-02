#pragma once
#include<vector>
#include"BackupManager/BackupConfig.h"
#include<functional>
#include"VectorTool.h"


#define _MAX(a,b) (((a)<(b))?(b):(a))


class BkTree {
public:
	struct BkNode {
		BackupFile file;
		std::vector<BkNode*> children;
		BkNode* parent = nullptr;
		int x = 0;
		int y = 0;
		int width = 0;
		int depth = 0;
		int timeLine = 0;
		BkNode(const BackupFile& file) :file(file) {}
		BkNode() {}
	};

	BkTree(BkNode* root = nullptr) :root(root) {}
	~BkTree() { removeAll(root); }

	void forEach(BkNode* node, const std::function<void(BkNode*)>& func, bool isChildFirst) {
		if (!node)return;
		if (!func)throw std::runtime_error("forEach: func is invalid!");
		if (!isChildFirst) {
			func(node);
		}
		for (auto& child : node->children) {
			forEach(child, func, isChildFirst);
		}
		if (isChildFirst) {
			func(node);
		}
	}

	void makeBranch(BkNode* newNode) {
		if (!current || !newNode)throw std::runtime_error("makeBranch: nullpointer exception!");
		current->children.push_back(newNode);
		newNode->parent = current;
		current = newNode;
	}

	void remove(BkNode* node) {
		if (!node)return;
		if (node->children.size() > 1 || !node->parent) {
			removeAll(node);
		}
		else {
			if (!node->children.empty()) {
				auto next = node->children.front();
				VectorTool::replace(node->parent->children, node, next);
				next->parent = node->parent;
			}
			else {
				VectorTool::erase(node->parent->children, node);
			}
			delete node;
		}
	}

	void removeAll(BkNode* node) {
		if (!node)return;
		forEach(node, [&](BkNode* current) {
			delete current;
		}, true);
	}

	void updatePosition(bool isDownwards = true) {
		if (!root)throw std::runtime_error("root is nullptr");
		endNodes.clear();
		forEach(root, [&](BkNode* node) {
			if (node->children.empty()) {
				endNodes.push_back(node);
			}
			if (node->parent) {
				node->depth = node->parent->depth + 1;
			}
			else {
				node->depth = 0;
			}
		}, false);
		int leftPosition = -endNodes.size() * nodeDeltaX / 2;
		for (int i = 0; i < endNodes.size(); i++) {
			auto& node = endNodes[i];
			node->y = node->depth * nodeDeltaY * (isDownwards ? 1 : -1);
			node->x = leftPosition + i * nodeDeltaX;
		}
		forEach(root, [&, isDownwards](BkNode* node) {
			if (!node)return;
			if (!node->children.empty()) {
				int temp = 0;
				for (auto& child : node->children) {
					temp += child->x;
				}
				node->x = temp / static_cast<int>(node->children.size());
				node->y = node->depth * nodeDeltaY * (isDownwards ? 1 : -1);
			}
		}, true);
	}

	BkNode* getRoot()const { return root; }

	void appendRandomly() {
		static bool isSeeded = false;
		if (!isSeeded) {
			srand(static_cast<unsigned>(time(nullptr)));
			isSeeded = true;
		}

		if (!root) {
			root = new BkNode();
			return;
		}

		std::vector<BkNode*> allNodes;
		collectAllNodes(root, allNodes);

		// 新的列表，优先级更高的节点将会被多次添加，增加被选中的概率
		std::vector<BkNode*> prioritizedNodes;

		// 为每个节点分配权重，权重由其子节点数量的倒数决定，没有子节点的权重最高
		for (auto* node : allNodes) {
			int weight = _MAX(1, (int)(20 - node->children.size())); // 确保每个节点至少有一个权重
			for (int i = 0; i < weight; i++) {
				prioritizedNodes.push_back(node);
			}
		}

		int randomIndex = rand() % prioritizedNodes.size();
		BkNode* parentNode = prioritizedNodes[randomIndex];

		BkNode* newNode = new BkNode();
		newNode->parent = parentNode;
		parentNode->children.push_back(newNode);
	}

	void collectAllNodes(BkNode* node, std::vector<BkNode*>& allNodes) {
		if (!node) return;
		allNodes.push_back(node);
		for (auto& child : node->children) {
			collectAllNodes(child, allNodes);
		}
	}

private:
	BkNode* root;
	BkNode* current;
	std::vector<BkNode*> endNodes;
	int nodeDeltaY = 90;				// 每一层的高度
	int nodeDeltaX = 70;			// 假设每个节点的宽度
	int mostDepth = 0;

};


#undef _MAX
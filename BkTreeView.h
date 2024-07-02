#pragma once
#include"BkTree.h"
#include<Qgraphicsscene>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QVBoxLayout>
#include <qevent.h>

class BkTreeView :public QGraphicsView {
	Q_OBJECT
private:
	BkTree* tree = nullptr;
	QGraphicsScene* scene = nullptr;

    void wheelEvent(QWheelEvent* event) override {
        const double scaleFactor = 1.15; // Zoom factor
        if (event->angleDelta().y() > 0) {
            scale(scaleFactor, scaleFactor);
        }
        else {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }

    ~BkTreeView() override {
        delete tree;
    }

public:
	BkTreeView(QWidget* parent = nullptr) :QGraphicsView(parent) {
        scene = new QGraphicsScene(this);
        this->setScene(scene);
        this->setDragMode(QGraphicsView::ScrollHandDrag);

        if (!tree) {
            tree = new BkTree(new BkTree::BkNode());
            for (int i = 0; i < 10; i++) {
                tree->appendRandomly();
            }
            tree->updatePosition(false);
        }

        drawTree();
	}

    void drawTree() {
        if (!tree) return;
        const auto WIDTH = 40;
        const auto HEIGHT = 20;
        tree->forEach(tree->getRoot(), [&](auto* node) {
            if (node->parent) {
                scene->addLine(node->parent->x, node->parent->y, node->x, node->y);
            }
            QGraphicsRectItem* rect = scene->addRect(node->x - WIDTH / 2, node->y - HEIGHT / 2, WIDTH, HEIGHT);
            QGraphicsTextItem* text = scene->addText("1");
            text->setPos(node->x - WIDTH / 2, node->y - HEIGHT / 2);
        }, false);
    }
};


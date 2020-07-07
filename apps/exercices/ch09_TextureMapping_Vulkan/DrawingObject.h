#ifndef DRAWINGOBJECT_H
#define DRAWINGOBJECT_H

#include <Node.h>

struct DrawingObject
{
    Material*          mat = nullptr;
    std::vector<Node*> nodeList;
};

#endif

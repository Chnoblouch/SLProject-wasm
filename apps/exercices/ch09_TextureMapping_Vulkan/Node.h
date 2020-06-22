#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <Object.h>
#include <Material.h>
#include <Mesh.h>
#include <SLMat4.h>

using namespace std;
//-----------------------------------------------------------------------------
class Node : public Object
{
public:
    Node(string name) : Object(name) { ; }

    void AddMesh(Mesh* mesh);
    void AddChild(Node* child);

protected:
    Node*         _parent;   //!< pointer to the parent node
    vector<Node*> _children; //!< vector of children nodes
    VMesh         _meshes;   //!< vector of meshes of the node
    SLMat4f       _om;       //!< object matrix for local transforms
};
//-----------------------------------------------------------------------------
typedef vector<Node> VNode;
//-----------------------------------------------------------------------------
#endif // NODE_H

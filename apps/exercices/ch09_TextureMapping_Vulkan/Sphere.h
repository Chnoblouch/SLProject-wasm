#pragma once

#include <string>

#include <Mesh.h>

using namespace std;
//-----------------------------------------------------------------------------
class Sphere : public Mesh
{

public:
    Sphere(string name) : Mesh(name) { build(); }

protected:
    float _radius; //!< radius of sphere
    int   _stacks; //!< NO. of stacks
    int   _slices; //!< NO. of slices

private:
    void build();
};
//-----------------------------------------------------------------------------

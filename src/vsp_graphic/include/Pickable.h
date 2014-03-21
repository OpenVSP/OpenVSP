#ifndef _VSP_GRAPHIC_PICKABLE_H
#define _VSP_GRAPHIC_PICKABLE_H

#include "SceneObject.h"

#include <string>

namespace VSPGraphic
{
class ColorBuffer;
class Renderable;

class Pickable : public SceneObject
{
public:
    Pickable(Renderable * source);
    virtual ~Pickable();

public:
    virtual bool processPickingResult(unsigned int pickedId);

public:
    Renderable * getSource();

public:
    void setGroup(std::string group);
    std::string getGroup();

public:
    virtual void update() = 0;

protected:
    virtual void _genColorBlock(bool single);
    virtual void _delColorBlock();

protected:
    Renderable * _rSource;
    ColorBuffer * _cIndexBuffer;

    bool _highlighted;

    std::string _groupName;

    struct
    {
        unsigned int start;
        unsigned int end;
    } _colorIndexRange;
};
}
#endif
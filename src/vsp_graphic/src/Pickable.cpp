#include "Pickable.h"

#include "Renderable.h"
#include "ColorCoder.h"
#include "VertexBuffer.h"
#include "ColorBuffer.h"

namespace VSPGraphic
{
Pickable::Pickable(Renderable * source) : SceneObject()
{
    _rSource = source;

    _cIndexBuffer = new ColorBuffer();

    _highlighted = false;

    _groupName = "";

    _colorIndexRange.start = 0;
    _colorIndexRange.end = 0;

    enablePredraw(true);
}
Pickable::~Pickable()
{
    delete _cIndexBuffer;
}

bool Pickable::processPickingResult(unsigned int pickedId)
{
    _highlighted = false;
    if(pickedId >= _colorIndexRange.start && pickedId <= _colorIndexRange.end)
    {
        _highlighted = true;
    }
    return _highlighted;
}

Renderable * Pickable::getSource()
{
    return _rSource;
}

void Pickable::setGroup(std::string groupName)
{
    _groupName = groupName;
}

std::string Pickable::getGroup()
{
    return _groupName;
}

void Pickable::_genColorBlock(bool geomPicking)
{
    // Just in case this function is spammed without _delColorBlock(),
    // free block before create.
    _delColorBlock();

    int vbSize = _rSource->getVBuffer()->getVertexSize();

    // Allocate enough space for the block (four bytes per vertex).
    std::vector<unsigned char> colorblock;
    colorblock.resize(vbSize * 4);

    if(!geomPicking)
    {
        // Create unique block of color indices.
        ColorCoderSingle.genCodeBlock(vbSize, &_colorIndexRange.start, &_colorIndexRange.end, colorblock.data());
    }
    else
    {
        std::vector<unsigned char> block;
        block.resize(4);

        // Create a single unique index for the whole block.
        ColorCoderSingle.genCodeBlock(1, &_colorIndexRange.start, &_colorIndexRange.end, block.data());

        for(int i = 0; i < vbSize; i++)
        {
            colorblock[i * 4 + 0] = block[0];
            colorblock[i * 4 + 1] = block[1];
            colorblock[i * 4 + 2] = block[2];
            colorblock[i * 4 + 3] = block[3];
        }
    }

    // Bind color indices to color buffer.
    _cIndexBuffer->empty();
    _cIndexBuffer->append(colorblock.data(), colorblock.size());
}

void Pickable::_delColorBlock()
{
    if(_colorIndexRange.start != 0 && _colorIndexRange.end != 0)
    {
        ColorCoderSingle.freeCodeBlock(_colorIndexRange.start, _colorIndexRange.end);
    }

    // Clean up
    _colorIndexRange.start = _colorIndexRange.end = 0;
    _cIndexBuffer->empty();
}
}

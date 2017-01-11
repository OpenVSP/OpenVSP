#include "PickablePnts.h"
#include "Renderable.h"
#include "ColorBuffer.h"
#include "VertexBuffer.h"

namespace VSPGraphic
{
PickablePnts::PickablePnts(Renderable * source) : Pickable(source)
{
    _pickRange = 20.0f;
    _pointSize = 10.0f;
}
PickablePnts::~PickablePnts()
{
    _delColorBlock();
}

bool PickablePnts::processPickingResult(unsigned int pickedId)
{
    bool flag = Pickable::processPickingResult(pickedId);


    if( flag )
    {
        _highlightedId.insert( pickedId );
    }

    return _highlighted;
}

std::vector< int > PickablePnts::getIndex()
{
    std::vector< int > vec;

    if(_highlightedId.size() == 0)
       return vec;


    std::set<int>::iterator it;
    vec.resize( _highlightedId.size() );
    int i = 0;
    for ( it = _highlightedId.begin(); it != _highlightedId.end(); ++it )
    {
        vec[i] = (*it) - _colorIndexRange.start;
        ++i;
    }

   return vec;
}

void PickablePnts::reset()
{
    _highlightedId.clear();
    _highlighted = false;
}

std::vector<glm::vec3> PickablePnts::getAllPnts()
{
    std::vector<glm::vec3> vertList;
    int numOfVert = _rSource->getVBuffer()->getVertexSize();

    for(int i = 0; i < numOfVert; i++)
    {
        vertList.push_back(_rSource->getVertexVec(i));
    }
    return vertList;
}

void PickablePnts::update()
{
    _genColorBlock(false);
}

void PickablePnts::_predraw()
{
    glPointSize(_pickRange);

    _cIndexBuffer->bind();
    _rSource->getVBuffer()->draw(GL_POINTS);
    _cIndexBuffer->unbind();
}

void PickablePnts::_draw()
{
    glColor3f(0.f, 1.f, 0.f);
    glPointSize(_pointSize);

    _rSource->getVBuffer()->draw(GL_POINTS);

    if(_highlighted)
    {
        std::vector< int > index = getIndex();
        for ( int i = 0; i < index.size(); i++ )
        {

            glm::vec3 hlPoint = _rSource->getVertexVec( index[i] );

            if(hlPoint != glm::vec3(0xFFFFFFFF))
            {
                glColor3f(1.f, 0.f, 0.f);
                glPointSize(_pointSize * 1.2f);
                glBegin(GL_POINTS);
                glVertex3f(hlPoint[0], hlPoint[1], hlPoint[2]);
                glEnd();
            }
        }

        // reset highlights so highlighted point turns off
        // after mouse moved away.
        _highlighted = false;
    }
}

void PickablePnts::setPickRange(float range)
{
    _pickRange = range;
}

void PickablePnts::setPointSize(float size)
{
    _pointSize = size;
}
}

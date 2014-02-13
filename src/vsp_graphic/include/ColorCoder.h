#ifndef _VSP_GRAPHIC_COLOR_CODE_GENERATOR_UTIL_H
#define _VSP_GRAPHIC_COLOR_CODE_GENERATOR_UTIL_H

#include <vector>

namespace VSPGraphic
{
/*!
* ColorCoder Class.
* This class is used to generate unique color IDs for Color Picking.
*/
class ColorCoder
{
public:
    /*!
    * Constructor.
    */
    ColorCoder();
    /*!
    * Destructor.
    */
    virtual ~ColorCoder();

public:
    /*!
    * Generate a block of unique color IDs.
    * size - number of color IDs.
    * start_out - block's starting ID out.
    * end_out - block's ending ID out.
    * block_out - byte array for color IDs.  Use this with Color Buffer.
    */
    void genCodeBlock( int size, unsigned int * start_out, unsigned int * end_out, unsigned char * block_out );
    /*!
    * Free a block of unique color IDs.  The freed IDs will be recycled.
    */
    void freeCodeBlock( unsigned int start, unsigned int end );

private:
    struct _freeblock
    {
        int start;
        int end;
    };
    std::vector<_freeblock> _freelist;
};
}
#endif
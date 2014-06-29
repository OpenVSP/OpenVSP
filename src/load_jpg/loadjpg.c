//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "loadjpg.h"




JpgData loadJPEG( const char* fileName )
{

    JpgData img;
    FILE              *fp;        // File pointer
    struct jpeg_decompress_struct cinfo;      // Decompressor info
    struct jpeg_error_mgr     jerr;       // Error handler info
    JSAMPROW          row;        // Sample row pointer

    img.w = 0;
    img.h = 0;
    img.d = 0;
    img.data  = 0;

    if ( ( fp = fopen( fileName, "rb" ) ) == NULL )
    {
        return img;
    }

    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_decompress( &cinfo );
    jpeg_stdio_src( &cinfo, fp );
    jpeg_read_header( &cinfo, 1 );

    cinfo.quantize_colors      = ( boolean )FALSE;
    cinfo.out_color_space      = JCS_RGB;
    cinfo.out_color_components = 3;
    cinfo.output_components    = 3;

    jpeg_calc_output_dimensions( &cinfo );

    img.w = cinfo.output_width;
    img.h = cinfo.output_height;
    img.d = cinfo.output_components;

    img.data = ( unsigned char* )malloc( sizeof( unsigned char ) * img.w * img.h * img.d );

    jpeg_start_decompress( &cinfo );

    while ( cinfo.output_scanline < cinfo.output_height )
    {
        row = ( JSAMPROW )( img.data + cinfo.output_scanline * img.w * img.d );
        jpeg_read_scanlines( &cinfo, &row, ( JDIMENSION )1 );
    }

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );

    fclose( fp );


    return img;


}

int writeJPEG( const char* filename, int w, int h, unsigned char* data )
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JSAMPROW row_pointer[1];
    FILE *outfile = fopen( filename, "wb" );

    if ( !outfile )
    {
        printf( "Error opening output jpeg file %s\n!", filename );
        return -1;
    }
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress( &cinfo );
    jpeg_stdio_dest( &cinfo, outfile );

    cinfo.image_width = w;
    cinfo.image_height = h;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults( &cinfo );
    jpeg_set_quality( &cinfo, 100, TRUE );  // Default is 75, TRUE

    jpeg_start_compress( &cinfo, TRUE );

    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &data[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
    fclose( outfile );

    return 1;

}



void deleteJPEG( JpgData img )
{
    if ( img.data )
    {
        free( img.data );
        img.data = 0;
    }
}






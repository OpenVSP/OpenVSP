#ifndef VSP_write_matlab_h
#define VSP_write_matlab_h

#include <vector>
#include <string>
#include "vec3d.h"
#include <cfloat>

class WriteMatlab
{
public:
    virtual ~WriteMatlab()
    {
    }

    virtual void write( FILE* file_id, const string &name )
    {
        fprintf( file_id, "%s = %.*e;\n", name.c_str(), DBL_DIG + 3, get() );
    }

    virtual double get() = 0;
};

class WriteDoubleM : public WriteMatlab
{
public:
    virtual void write( FILE* file_id, const double &d, const string &name )
    {
        data = d;
        WriteMatlab::write( file_id, name );
    }

    double get()
    {
        return data;
    }

protected:
    double data;
};

class WriteVec3dM : public WriteMatlab
{
public:
    virtual void write( FILE* file_id, const vec3d &d, const string &basename )
    {
        data = d;
        string suffix[] = {"x", "y", "z"};
        for( dim = 0; dim < 3; dim++ )
        {
            string name = basename;
            name.append( suffix[dim] );
            WriteMatlab::write( file_id, name );
        }
    }

    double get()
    {
        return data.v[dim];
    }

protected:
    vec3d data;
    int dim;
};

class WriteMatlabVec
{
public:
    virtual ~WriteMatlabVec()
    {
    }

    virtual void write( FILE* file_id, const string &name, const int &num )
    {
        int i;
        fprintf( file_id, "\n%s = [", name.c_str() );

        for ( i = 0; i < num - 1; i++ )
        {
            fprintf( file_id, "%.*e;\n", DBL_DIG + 3, get( i ) );
        }

        fprintf( file_id, "%.*e];\n", DBL_DIG + 3, get( i ) );
    }

    virtual double get( int i ) = 0;
};

class WriteVecDoubleM : public WriteMatlabVec
{
public:
    virtual void write( FILE* file_id, const vector< double > &d, const string &name, const int &num )
    {
        data = d;
        WriteMatlabVec::write( file_id, name, num );
    }

    double get( int i )
    {
        return data[i];
    }

protected:
    vector< double > data;
};

class WriteVecVec3dM : public WriteMatlabVec
{
public:
    virtual void write( FILE* file_id, const vector< vec3d > &d, const string &basename, const int &num )
    {
        data = d;
        string suffix[] = {"x", "y", "z"};
        for( dim = 0; dim < 3; dim++ )
        {
            string name = basename;
            name.append( suffix[dim] );
            WriteMatlabVec::write( file_id, name, num );
        }
    }

    double get( int i )
    {
        return data[i].v[dim];
    }

protected:
    vector< vec3d > data;
    int dim;
};

class WriteMatlabMat
{
public:
    virtual ~WriteMatlabMat()
    {
    }

    virtual void write( FILE* file_id, const string &name, const int &numi, const int &numj )
    {
        int i, j;

        fprintf( file_id, "\n%s = [", name.c_str() );
        for ( i = 0; i < numi; i++ )
        {
            for ( j = 0; j < numj - 1; j++ )
            {
                fprintf( file_id, "%.*e, ", DBL_DIG + 3, get( i, j ) );
            }
            if ( i < numi - 1 )
            {
                fprintf( file_id, "%.*e;\n", DBL_DIG + 3, get( i, j ) );
            }
            else
            {
                fprintf( file_id, "%.*e];\n", DBL_DIG + 3, get( i, j ) );
            }
        }
    }

    virtual double get( int i, int j ) = 0;
};

class WriteMatDoubleM : public WriteMatlabMat
{
public:
    virtual void write( FILE* file_id, const vector< vector< double > > &d, const string &name, const int &numi, const int &numj )
    {
        data = d;
        WriteMatlabMat::write( file_id, name, numi, numj );
    }

    double get( int i, int j )
    {
        return data[i][j];
    }

protected:
    vector< vector< double > > data;
};

class WriteMatVec3dM : public WriteMatlabMat
{
public:
    virtual void write( FILE* file_id, const vector< vector< vec3d > > &d, const string &basename, const int &numi, const int &numj )
    {
        data = d;
        string suffix[] = {"x", "y", "z"};
        for( dim = 0; dim < 3; dim++ )
        {
            string name = basename;
            name.append( suffix[dim] );
            WriteMatlabMat::write( file_id, name, numi, numj );
        }
    }

    double get( int i, int j )
    {
        return data[i][j].v[dim];
    }

protected:
    vector< vector< vec3d > > data;
    int dim;
};

#endif

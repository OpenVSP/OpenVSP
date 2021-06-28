#ifndef VSPAERO_TYPES_H
#define VSPAERO_TYPES_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>

int FWRITE(int    *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(float  *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(double *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(char   *Value, size_t Size, size_t Num, FILE *File);

int FREAD(int     *Value, size_t Size, size_t Num, FILE *File);
int FREAD(float   *Value, size_t Size, size_t Num, FILE *File);
int FREAD(double  *Value, size_t Size, size_t Num, FILE *File);
int FREAD(char    *Value, size_t Size, size_t Num, FILE *File);

float FLOAT(double a);
double DOUBLE(double a);

template <typename T>
T Argument(T value) noexcept
{
  return value;
}

template <typename T>
T const * Argument(std::basic_string<T> const & value) noexcept
{
  return value.c_str();
}

///////////////////////////////////// Automatic Differentiation Stuff ///////////////////////////////////////////

#ifdef AUTODIFF

#include "adept.h"

#define VSPAERO_DOUBLE adept::adouble

float FLOAT(adept::adouble a);
double DOUBLE(adept::adouble a);

int FWRITE(adept::adouble *Value, size_t Size, size_t Num, FILE *File);

int FREAD(adept::adouble  *Value, size_t Size, size_t Num, FILE *File);

double const  Argument(adept::adouble const & value) noexcept;

// OPENMP stuff for Codi

#define DO_PARALLEL_LOOP 0

#pragma omp declare reduction(+: adept::adouble: \
     omp_out.set_gradient( omp_out.get_gradient() + omp_in.get_gradient() ), \
     omp_out.set_value( omp_out.value() + omp_in.value()))\
     initializer (omp_priv=adept::adouble(0.0))
     
///////////////////////////////////// NORMAL Stuff... ///////////////////////////////////////////////////////////

#else

#define VSPAERO_DOUBLE double

#define DO_PARALLEL_LOOP 1

#endif

template <typename ... Args>
void PRINTF(char const * const format,
            Args const & ... args) noexcept
{
  printf(format, Argument(args) ...);
}

template <typename ... Args>
void FPRINTF(FILE *stream, char const * const format,
            Args const & ... args) noexcept
{
  fprintf(stream, format, Argument(args) ...);
}

template <typename ... Args>
void SPRINTF(char *str, char const * const format,
            Args const & ... args) noexcept
{
  sprintf(str, format, Argument(args) ...);
}


#endif






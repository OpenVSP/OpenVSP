#ifndef VSPAERO_TYPES_H
#define VSPAERO_TYPES_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>

#ifdef AUTODIFF
#include "adept.h"
#endif 

#ifdef COMPLEXDIFF
#include <complex>
#endif

#include "START_NAME_SPACE.H"

int FWRITE(int    *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(float  *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(double *Value, size_t Size, size_t Num, FILE *File);
int FWRITE(char   *Value, size_t Size, size_t Num, FILE *File);

int FREAD(int     *Value, size_t Size, size_t Num, FILE *File);
int FREAD(float   *Value, size_t Size, size_t Num, FILE *File);
int FREAD(double  *Value, size_t Size, size_t Num, FILE *File);
int FREAD(char    *Value, size_t Size, size_t Num, FILE *File);

int INTEGER(double a);
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

#define AUTODIFF_IS_OFF 0


#define VSPAERO_DOUBLE adept::adouble

#define PARALLEL(x) " "

int INTEGER(adept::adouble a);
float FLOAT(adept::adouble a);
double DOUBLE(adept::adouble a);

int FWRITE(adept::adouble *Value, size_t Size, size_t Num, FILE *File);

int FREAD(adept::adouble  *Value, size_t Size, size_t Num, FILE *File);

double const  Argument(adept::adouble const & value) noexcept;

//#pragma omp declare reduction(+: adept::adouble: \
//     omp_out.set_gradient( omp_out.get_gradient() + omp_in.get_gradient() ), \
//     omp_out.set_value( omp_out.value() + omp_in.value()))\
//     initializer (omp_priv=adept::adouble(0.0))
     
///////////////////////////////////// COMPLEX STEP Stuff... ///////////////////////////////////////////////////////////

#elif COMPLEXDIFF

#define AUTODIFF_IS_OFF 1

#define VSPAERO_DOUBLE std::complex<double>

int INTEGER(VSPAERO_DOUBLE a);
float FLOAT(VSPAERO_DOUBLE a);
double DOUBLE(VSPAERO_DOUBLE a);

int FWRITE(VSPAERO_DOUBLE *Value, size_t Size, size_t Num, FILE *File);

int FREAD(VSPAERO_DOUBLE  *Value, size_t Size, size_t Num, FILE *File);

double const  Argument(VSPAERO_DOUBLE const & value) noexcept;

int operator> (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b);
int operator< (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b);

int operator>= (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b);
int operator<= (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b);

VSPAERO_DOUBLE operator* (int a,VSPAERO_DOUBLE b);   
VSPAERO_DOUBLE operator* (VSPAERO_DOUBLE a,int b);

VSPAERO_DOUBLE operator* (double a,VSPAERO_DOUBLE b);
VSPAERO_DOUBLE operator* (VSPAERO_DOUBLE a,double b);

VSPAERO_DOUBLE operator/ (int a,VSPAERO_DOUBLE b);   
VSPAERO_DOUBLE operator/ (VSPAERO_DOUBLE a,int b);

//VSPAERO_DOUBLE operator/ (double a,VSPAERO_DOUBLE b);
VSPAERO_DOUBLE operator/ (VSPAERO_DOUBLE a,double b);

float operator+ (float a,VSPAERO_DOUBLE b);
float operator+ (VSPAERO_DOUBLE a,float b);
      
double operator+ (double a,VSPAERO_DOUBLE b);
double operator+ (VSPAERO_DOUBLE a,double b);

double INIT_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X);
void ZERO_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X);
double CALCULATE_COMPLEX_DIFF_FOR_FUNCTION(VSPAERO_DOUBLE F, double Delta);

void INIT_1_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X, double Delta);
void INIT_2_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X, double Delta);
double CALCULATE_COMPLEX_DIFF_FOR_FUNCTION(VSPAERO_DOUBLE F1, VSPAERO_DOUBLE F2, double Delta);

#pragma omp declare	\
reduction(	\
	+ : \
	std::complex<double> :	\
	omp_out += omp_in )	\
initializer( omp_priv = omp_orig )

///////////////////////////////////// NORMAL Stuff... ///////////////////////////////////////////////////////////

#else

#define AUTODIFF_IS_OFF 1

#define VSPAERO_DOUBLE double

#endif

///////////////////////////////////// PRINT STUFF... ///////////////////////////////////////////////////////////

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

///////////////////////////////////// AUTODIFF Stuff ///////////////////////////////////////////////////////////

//adept::Stack *CURRENT_STACK(void) { return adept::active_stack(); };

int AUTO_DIFF_IS_RECORDING(void);
void START_NEW_AUTO_DIFF(void);
void PAUSE_AUTO_DIFF(void);
void AUTO_DIFF_STACK_STATUS(void);
void CONTINUE_AUTO_DIFF(void);
void CALCULATE_ADJOINT(void);
void CLEAR_GRADIENTS(void);
void CLEAR_ALL_GRADIENT_DATA(void);
void SET_DEPENDENT_FUNCTION(VSPAERO_DOUBLE &Function);
void SET_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &Function);
void SET_GRADIENT(VSPAERO_DOUBLE &Function, double Value);
double GET_VALUE(VSPAERO_DOUBLE &Function);
double GET_GRADIENT(VSPAERO_DOUBLE &Function);
double AUTO_DIFF_STACK_MEMORY(void);

///////////////////////////////////// COMPLEX DIFF Stuff ///////////////////////////////////////////////////////////

#include "END_NAME_SPACE.H"

#endif

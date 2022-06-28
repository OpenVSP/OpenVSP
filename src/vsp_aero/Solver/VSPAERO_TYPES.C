#include "VSPAERO_TYPES.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                       Convert double to a float                              #
#                                                                              #
##############################################################################*/

int INTEGER(double a) { 
   
    return (int) a; 
    
}

/*##############################################################################
#                                                                              #
#                       Convert double to a float                              #
#                                                                              #
##############################################################################*/

float FLOAT(double a) { 
   
    return (float) a; 
    
}

/*##############################################################################
#                                                                              #
#                       Convert double to a double                             #
#                                                                              #
##############################################################################*/

double DOUBLE(double a) { 
   
    return (double) a; 
    
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(int *Value, size_t Size, size_t Num, FILE *File) {
   
   return fwrite(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(float *Value, size_t Size, size_t Num, FILE *File) {
   
   return fwrite(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(double *Value, size_t Size, size_t Num, FILE *File) {
   
   return fwrite(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(char *Value, size_t Size, size_t Num, FILE *File) {
   
   return fwrite(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(int *Value, size_t Size, size_t Num, FILE *File) {
   
   return fread(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(float *Value, size_t Size, size_t Num, FILE *File) {
   
   return fread(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(double *Value, size_t Size, size_t Num, FILE *File) {
   
   return fread(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(char *Value, size_t Size, size_t Num, FILE *File) {
   
   return fread(Value, Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                         AUTO_DIFF_STACK_STATUS                               #
#                                                                              #
##############################################################################*/

void AUTO_DIFF_STACK_STATUS(void) {

#ifdef AUTODIFF
   
    // Get stack status

    adept::active_stack()->print_status();
   
#endif      
   
}


/*##############################################################################
#                                                                              #
#                         AUTO_DIFF_STACKMEMORY                                #
#                                                                              #
##############################################################################*/

double AUTO_DIFF_STACK_MEMORY(void) {

#ifdef AUTODIFF
   
    // Get stack memory useage

    return (adept::active_stack()->memory()/(1.e9));

#else

    return 0;
    
#endif      
   
}

/*##############################################################################
#                                                                              #
#                         AUTO_DIFF_IS_RECORDING                               #
#                                                                              #
##############################################################################*/

int AUTO_DIFF_IS_RECORDING(void) {

#ifdef AUTODIFF
   
    // Check if we are recording

    return adept::active_stack()->is_recording();

#else

    return 0;
    
#endif      
   
}

/*##############################################################################
#                                                                              #
#                             START_NEW_AUTO_DIFF                              #
#                                                                              #
##############################################################################*/

void START_NEW_AUTO_DIFF(void) {
   
#ifdef AUTODIFF

    // Start a new recording

    if ( !(adept::active_stack()->is_recording()) ) adept::active_stack()->continue_recording();
    
    adept::active_stack()->new_recording();
      
#endif
   
}

/*##############################################################################
#                                                                              #
#                             PAUSE_AUTO_DIFF                                  #
#                                                                              #
##############################################################################*/

void PAUSE_AUTO_DIFF(void) {

#ifdef AUTODIFF
   
    // Pause recording

    adept::active_stack()->pause_recording();

#endif   
   
}

/*##############################################################################
#                                                                              #
#                             CONTINUE_AUTO_DIFF                               #
#                                                                              #
##############################################################################*/

void CONTINUE_AUTO_DIFF(void) {
 
#ifdef AUTODIFF
   
    // Continue recording
 
    adept::active_stack()->continue_recording();

#endif
                
}

/*##############################################################################
#                                                                              #
#                             CALCULATE_ADJOINT                                #
#                                                                              #
##############################################################################*/

void CALCULATE_ADJOINT(void) {
 
#ifdef AUTODIFF
   
    // Calculate adjoint
 
    adept::active_stack()->compute_adjoint();

#endif
                
}

/*##############################################################################
#                                                                              #
#                             CLEAR_GRADIENTS                                  #
#                                                                              #
##############################################################################*/

void CLEAR_GRADIENTS(void) {
 
#ifdef AUTODIFF
   
    // Clear all gradients
 
    adept::active_stack()->clear_gradients();

#endif
                
}

/*##############################################################################
#                                                                              #
#                         CLEAR_ALL_GRADIENT_DATA                              #
#                                                                              #
##############################################################################*/

void CLEAR_ALL_GRADIENT_DATA(void) {
 
#ifdef AUTODIFF
   
    // Clear all gradient data
    
    adept::active_stack()->clear_gradients();
    adept::active_stack()->clear_independents();
    adept::active_stack()->clear_dependents();    

#endif
                
}

/*##############################################################################
#                                                                              #
#                      SET_DEPENDENT_FUNCTION                                  #
#                                                                              #
##############################################################################*/

void SET_DEPENDENT_FUNCTION(VSPAERO_DOUBLE &Function) {
 
#ifdef AUTODIFF
   
    // Set dependent function
 
    adept::active_stack()->dependent(Function);

#endif
                
}

/*##############################################################################
#                                                                              #
#                      SET_INDEPENDENT_VARIABLE                                #
#                                                                              #
##############################################################################*/

void SET_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &Function) {
 
#ifdef AUTODIFF
   
    // Set independent function
 
    adept::active_stack()->independent(Function);

#endif
                
}

/*##############################################################################
#                                                                              #
#                              SET_GRADIENT                                    #
#                                                                              #
##############################################################################*/

void SET_GRADIENT(VSPAERO_DOUBLE &Function, double Value) {
 
#ifdef AUTODIFF
   
    // Set independent function
    
    Function.set_gradient(Value);

#endif
                
}

/*##############################################################################
#                                                                              #
#                               GET_VALUE                                      #
#                                                                              #
##############################################################################*/

double GET_VALUE(VSPAERO_DOUBLE &Function) {
 
#ifdef AUTODIFF

    return Function.value();

#elif COMPLEXDIFF

    return Function.real();

#else

    return Function;

#endif
                
}

/*##############################################################################
#                                                                              #
#                               GET_GRADIENT                                   #
#                                                                              #
##############################################################################*/

double GET_GRADIENT(VSPAERO_DOUBLE &Function) {
 
#ifdef AUTODIFF
   
    // Return gradient
    
    return Function.get_gradient();

#else

    return 0.;

#endif
                
}

//////////////////////////////////// AUTODIFF //////////////////////////////////
//////////////////////////////////// AUTODIFF //////////////////////////////////
//////////////////////////////////// AUTODIFF //////////////////////////////////
//////////////////////////////////// AUTODIFF //////////////////////////////////

#ifdef AUTODIFF

/*##############################################################################
#                                                                              #
#                       Convert RealReverse to a int                           #
#                                                                              #
##############################################################################*/

int INTEGER(adept::adouble a) { 
   
    return (int) a.value(); 
    
}

/*##############################################################################
#                                                                              #
#                  Convert RealReverse to a float                              #
#                                                                              #
##############################################################################*/
   
float FLOAT(adept::adouble a) {
   
    return (float) a.value();
    
}

/*##############################################################################
#                                                                              #
#                  Convert RealReverse to a double                              #
#                                                                              #
##############################################################################*/
   
double DOUBLE(adept::adouble a) {
   
    return (double) a.value();
    
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(adept::adouble *Value, size_t Size, size_t Num, FILE *File) {
   
   double DumDouble;
   
   DumDouble = Value->value();

   return fwrite(&(DumDouble), Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(adept::adouble *Value, size_t Size, size_t Num, FILE *File) {
   
   int DumInt;
   double DumDouble;

   DumInt = fread(&DumDouble, Size, Num, File);
   
   Value->set_value(DumDouble);

   return DumInt;
   
}

/*##############################################################################
#                                                                              #
#                             AUTODIFF Argument                                #
#                                                                              #
##############################################################################*/

double const Argument(adept::adouble const & value) noexcept
{

  return value.value();
  
}

#endif

//////////////////////////////////// COMPLEX STEP //////////////////////////////////
//////////////////////////////////// COMPLEX STEP //////////////////////////////////
//////////////////////////////////// COMPLEX STEP //////////////////////////////////
//////////////////////////////////// COMPLEX STEP //////////////////////////////////

#ifdef COMPLEXDIFF

/*##############################################################################
#                                                                              #
#                       Convert COMPLEX to a int                               #
#                                                                              #
##############################################################################*/

int INTEGER(VSPAERO_DOUBLE a) { 
   
    return (int) a.real(); 
    
}

/*##############################################################################
#                                                                              #
#                  Convert COMPLEX to a float                                  #
#                                                                              #
##############################################################################*/
   
float FLOAT(VSPAERO_DOUBLE a) {
   
    return (float) a.real();
    
}

/*##############################################################################
#                                                                              #
#                  Convert COMPLEX to a double                                 #
#                                                                              #
##############################################################################*/
   
double DOUBLE(VSPAERO_DOUBLE a) {
   
    return (double) a.real();
    
}

/*##############################################################################
#                                                                              #
#                                 FWRITE                                       #
#                                                                              #
##############################################################################*/

int FWRITE(VSPAERO_DOUBLE *Value, size_t Size, size_t Num, FILE *File) {
   
   double DumDouble;
   
   DumDouble = Value->real();

   return fwrite(&(DumDouble), Size, Num, File);
   
}

/*##############################################################################
#                                                                              #
#                                 FREAD                                        #
#                                                                              #
##############################################################################*/

int FREAD(VSPAERO_DOUBLE *Value, size_t Size, size_t Num, FILE *File) {
   
   int DumInt;
   double DumDouble;

   DumInt = fread(&DumDouble, Size, Num, File);
   
   Value->real(DumDouble);
   Value->imag(0.);

   return DumInt;
   
}

/*##############################################################################
#                                                                              #
#                             COMPLEX DIFF Argument                            #
#                                                                              #
##############################################################################*/

double const Argument(VSPAERO_DOUBLE const & value) noexcept
{

  return value.real();
  
}

/*##############################################################################
#                                                                              #
#                          COMPLEX DIFF operator>                              #
#                                                                              #
##############################################################################*/

int operator> (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b)
{

    if ( a.real() > b.real() ) {
       
       return 1;
       
    }
    
    else {
       
       return 0;
       
    }

}

/*##############################################################################
#                                                                              #
#                          COMPLEX DIFF operator<                              #
#                                                                              #
##############################################################################*/

int operator< (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b)
{

    if ( a.real() < b.real() ) {
       
       return 1;
       
    }
    
    else {
       
       return 0;
       
    }

}

/*##############################################################################
#                                                                              #
#                          COMPLEX DIFF operator>=                             #
#                                                                              #
##############################################################################*/

int operator>= (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b)
{

    if ( a.real() >= b.real() ) {
       
       return 1;
       
    }
    
    else {
       
       return 0;
       
    }

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator<=                                 #
#                                                                              #
##############################################################################*/

int operator<= (VSPAERO_DOUBLE a,VSPAERO_DOUBLE b)
{

    if ( a.real() <= b.real() ) {
       
       return 1;
       
    }
    
    else {
       
       return 0;
       
    }

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator*                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator* (int a,VSPAERO_DOUBLE b)
{
   
    VSPAERO_DOUBLE result;

    result.real( a * b.real() );
    result.imag( a * b.imag() );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator*                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator* (double a,VSPAERO_DOUBLE b)
{
   
    VSPAERO_DOUBLE result;

    result.real( a * b.real() );
    result.imag( a * b.imag() );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator*                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator* (VSPAERO_DOUBLE a, int b)
{
   
    VSPAERO_DOUBLE result;

    result.real( b * a.real() );
    result.imag( b * a.imag() );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator*                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator* (VSPAERO_DOUBLE a, double b)
{
   
    VSPAERO_DOUBLE result;

    result.real( b * a.real() );
    result.imag( b * a.imag() );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator/                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator/ (int a,VSPAERO_DOUBLE b)
{
   
    VSPAERO_DOUBLE ac;
    
    ac.real(DOUBLE(a));
    ac.imag(0.);

    return ac/b;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator/                                  #
#                                                                              #
##############################################################################*/

//VSPAERO_DOUBLE operator/ (double a, VSPAERO_DOUBLE b)
//{
//   
//    VSPAERO_DOUBLE result;
//
//    result.real( b.real() / a );
//    result.imag( b.imag() / a );
//    
//    return result;
//
//}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator/                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator/ (VSPAERO_DOUBLE a, int b)
{
   
    VSPAERO_DOUBLE result;

    result.real( a.real() / b );
    result.imag( a.imag() / b );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator/                                  #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE operator/ (VSPAERO_DOUBLE a, double b)
{
   
    VSPAERO_DOUBLE result;

    result.real( a.real() / b );
    result.imag( a.imag() / b );
    
    return result;

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator+                                  #
#                                                                              #
##############################################################################*/

double operator+ (double a, VSPAERO_DOUBLE b)
{

    return a + b.real();

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator+                                  #
#                                                                              #
##############################################################################*/

double operator+ (VSPAERO_DOUBLE a, double b)
{

    return b + a.real();

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator+                                  #
#                                                                              #
##############################################################################*/

float operator+ (float a, VSPAERO_DOUBLE b)
{

    return a + b.real();

}

/*##############################################################################
#                                                                              #
#                      COMPLEX DIFF operator+                                  #
#                                                                              #
##############################################################################*/

float operator+ (VSPAERO_DOUBLE a, float b)
{

    return b + a.real();

}

/*##############################################################################
#                                                                              #
#               INIT_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE                     #
#                                                                              #
##############################################################################*/

double INIT_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X)
{
   
    double Delta;

    Delta = 1.e-20;

    X.imag(Delta);
    
    return Delta;
   
}

/*##############################################################################
#                                                                              #
#               ZERO_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE                     #
#                                                                              #
##############################################################################*/

void ZERO_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X)
{

    X.imag(0.);

}

/*##############################################################################
#                                                                              #
#                CALCULATE_COMPLEX_DIFF_FOR_FUNCTION                           #
#                                                                              #
##############################################################################*/

double CALCULATE_COMPLEX_DIFF_FOR_FUNCTION(VSPAERO_DOUBLE F, double Delta)
{
   
    return F.imag() / Delta;
   
}

/*##############################################################################
#                                                                              #
#                INIT_1_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE                  #
#                                                                              #
##############################################################################*/

void INIT_1_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X, double Delta)
{

    double Xr, Xi;
    
  //  Xr = ( X.real() + 0.5          )*Delta;
  //  Xi = ( 0.0      + 0.5*sqrt(3.) )*Delta;
  //  
  //  X.real(Xr);
  //  X.imag(Xi);
    
    X.imag(Delta);
        
    
}

/*##############################################################################
#                                                                              #
#                INIT_2_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE                  #
#                                                                              #
##############################################################################*/

void INIT_2_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE(VSPAERO_DOUBLE &X, double Delta)
{

    double Xr, Xi;
    
  //  Xr = ( X.real() - 0.5          )*Delta;
  //  Xi = ( 0.0      - 0.5*sqrt(3.) )*Delta;
  //  
  //  X.real(Xr);
  //  X.imag(Xi);
    
    X.imag(-Delta);
    
    
}

/*##############################################################################
#                                                                              #
#                INIT_2_COMPLEX_DIFF_FOR_INDEPENDENT_VARIABLE                  #
#                                                                              #
##############################################################################*/

double CALCULATE_COMPLEX_DIFF_FOR_FUNCTION(VSPAERO_DOUBLE F1, VSPAERO_DOUBLE F2, double Delta)
{

    VSPAERO_DOUBLE Fp;
    
  //  Fp = ( F1 - F2 ) / ( sqrt(3)*Delta );
    
    Fp = ( F1 - F2 )/(2.*Delta);
    
    return Fp.imag();
    
}

#endif

#include "END_NAME_SPACE.H"



#include "VSPAERO_TYPES.H"

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

//////////////////////////////////// AUTODIFF //////////////////////////////////

#ifdef AUTODIFF

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





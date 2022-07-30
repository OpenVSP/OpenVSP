//
//  xcodeLog.mm
//
//  Created by Jordi Oliveras Rovira on 04/04/14.
//

#import <stdarg.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSString.h>

int printfXcode(const char * __restrict format, ...)
{
    va_list args;
    va_start(args,format);
    NSLogv([NSString stringWithUTF8String:format], args) ;
    va_end(args);
    return 1;
}

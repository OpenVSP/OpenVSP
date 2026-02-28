#import <Cocoa/Cocoa.h>
#include <unistd.h>

static NSString *pendingFile;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (BOOL)application:(NSApplication *)app openFile:(NSString *)path {
    pendingFile = path;
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)n {
    NSString *dir = [[NSBundle mainBundle] resourcePath];
    NSString *vsp = [dir stringByAppendingPathComponent:@"vsp"];

    NSString *file = pendingFile;
    if (!file) {
        NSArray *args = [[NSProcessInfo processInfo] arguments];
        if (args.count > 1) file = args[1];
    }

    if (file && ![file hasPrefix:@"/"]) {
        char *cwd = getcwd(NULL, 0);
        if (cwd) {
            file = [[NSString stringWithUTF8String:cwd]
                        stringByAppendingPathComponent:file];
            free(cwd);
        }
    }

    chdir([dir fileSystemRepresentation]);

    if (file)
        execl([vsp fileSystemRepresentation], "vsp",
              [file fileSystemRepresentation], NULL);
    else
        execl([vsp fileSystemRepresentation], "vsp", NULL);

    perror("execl");
    _exit(1);
}

@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        app.delegate = [[AppDelegate alloc] init];
        [app run];
    }
    return 0;
}

#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <QuartzCore/CAMetalLayer.h>
#import "ios_app.h"
#include <sys/types.h>

// JIT detection: a debugger (StikDebug / SideStore / Xcode) sets CS_DEBUGGED on the process.
extern "C" int csops(pid_t pid, unsigned int ops, void* useraddr, size_t usersize);
static bool jit_enabled()
{
	int flags = 0;
	csops(getpid(), 0 /*CS_OPS_STATUS*/, &flags, sizeof(flags));
	return (flags & 0x10000000 /*CS_DEBUGGED*/) != 0;
}

@interface MetalView : UIView
@end
@implementation MetalView
+ (Class)layerClass { return [CAMetalLayer class]; }
@end

@interface RPCS3ViewController : UIViewController <UIDocumentPickerDelegate>
@property (strong) MetalView* metalView;
@property (strong) UIButton* bootButton;
@property (strong) UILabel* status;
@end

@implementation RPCS3ViewController

- (void)viewDidLoad
{
	[super viewDidLoad];
	self.view.backgroundColor = UIColor.blackColor;

	self.metalView = [[MetalView alloc] initWithFrame:self.view.bounds];
	self.metalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	self.metalView.contentScaleFactor = UIScreen.mainScreen.nativeScale;
	[self.view addSubview:self.metalView];
	rpcs3_ios_set_view((__bridge void*)self.metalView);

	self.status = [[UILabel alloc] initWithFrame:CGRectMake(20, 40, self.view.bounds.size.width - 40, 60)];
	self.status.textColor = UIColor.whiteColor;
	self.status.numberOfLines = 3;
	self.status.autoresizingMask = UIViewAutoresizingFlexibleWidth;
	self.status.text = jit_enabled() ? @"JIT enabled. Pick a game (EBOOT.BIN / .iso / folder)."
	                                 : @"JIT NOT enabled. Attach via StikDebug/SideStore, then relaunch.";
	[self.view addSubview:self.status];

	self.bootButton = [UIButton buttonWithType:UIButtonTypeSystem];
	self.bootButton.frame = CGRectMake(20, 110, 220, 44);
	[self.bootButton setTitle:@"Boot game…" forState:UIControlStateNormal];
	[self.bootButton addTarget:self action:@selector(pick) forControlEvents:UIControlEventTouchUpInside];
	[self.view addSubview:self.bootButton];
}

- (void)pick
{
	NSArray* types = @[UTTypeFolder, UTTypeData];
	UIDocumentPickerViewController* p = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:types asCopy:NO];
	p.delegate = self;
	[self presentViewController:p animated:YES completion:nil];
}

- (void)documentPicker:(UIDocumentPickerViewController*)c didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls
{
	NSURL* url = urls.firstObject;
	[url startAccessingSecurityScopedResource]; // keep open for the emulator's lifetime
	self.bootButton.hidden = YES;
	self.status.hidden = YES;
	std::string path = url.path.UTF8String;
	dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
		const bool ok = rpcs3_ios_boot(path.c_str());
		if (!ok)
		{
			dispatch_async(dispatch_get_main_queue(), ^{
				self.bootButton.hidden = NO;
				self.status.hidden = NO;
				self.status.text = @"Boot failed. See RPCS3.log in the app's Documents folder (Files app).";
			});
		}
	});
}

- (BOOL)prefersStatusBarHidden { return YES; }
- (BOOL)prefersHomeIndicatorAutoHidden { return YES; }
- (UIInterfaceOrientationMask)supportedInterfaceOrientations { return UIInterfaceOrientationMaskLandscape; }
@end

@implementation RPCS3AppDelegate
- (void)applicationDidEnterBackground:(UIApplication*)app { rpcs3_ios_flush_logs(); }
- (void)applicationWillTerminate:(UIApplication*)app { rpcs3_ios_flush_logs(); }

- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)opts
{
	NSString* docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
	rpcs3_ios_init(docs.UTF8String);

	self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
	self.window.rootViewController = [RPCS3ViewController new];
	[self.window makeKeyAndVisible];
	return YES;
}
@end

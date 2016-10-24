/*
	SRCViewFactory.h
	SecretRabbit Sample Rate Converter AU
	
	Kevin C. Dixon
	Yano Signal Processing
	10/04/2008
*/

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>

@class SRCView;


@interface SRCViewFactory : NSObject <AUCocoaUIBase>
{
    IBOutlet SRCView * uiFreshlyLoadedView;
}

- (NSString *) description;	// string description of the view

@end
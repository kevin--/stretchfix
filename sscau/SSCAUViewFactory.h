/*
SSCAU - SMPTE-Stretch-Calc Audio Unit
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/
*/
/*
	SSCAUViewFactory.h
	
	02/16/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>

@class SSCAUView;


@interface SSCAUViewFactory : NSObject<AUCocoaUIBase>
{
	IBOutlet SSCAUView * uiFreshlyLoadedView;
}

- (NSString *) description;	// string description of the view

@end

/*
SSCAU - SMPTE-Stretch-Calc Audio Unit
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*
	SSCAUViewFactory.m
	
	02/16/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#import "SSCAUViewFactory.h"
#import "SSCAUView.h"

@implementation SSCAUViewFactory

// version 0
- (unsigned) interfaceVersion
{
	return 0;
}

// string description of the Cocoa UI
- (NSString *) description
{
	return @" Cocoa View";
}

// N.B.: this class is simply a view-factory,
// returning a new autoreleased view each time it's called.
- (NSView *)uiViewForAudioUnit:(AudioUnit)inAU withSize:(NSSize)inPreferredSize
{

	if (! [NSBundle loadNibNamed: @"CocoaView" owner:self])
	{
		NSLog (@"Unable to load nib for view.");
		return nil;
	}
    
    // This particular nib has a fixed size, so we don't do anything with the inPreferredSize argument.
    // It's up to the host application to handle.
    [uiFreshlyLoadedView setAU:inAU];
    
    NSView *returnView = uiFreshlyLoadedView;
    uiFreshlyLoadedView = nil;	// zero out pointer.  This is a view factory.  Once a view's been created
                                // and handed off, the factory keeps no record of it.
    
    return [returnView autorelease];
}

@end

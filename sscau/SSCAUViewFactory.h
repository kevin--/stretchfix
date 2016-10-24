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
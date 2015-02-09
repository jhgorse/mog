///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AppleAVListEnumerator.mm
///
/// Copyright (c) 2014, BoxCast, Inc. All rights reserved.
///
/// This library is free software; you can redistribute it and/or modify it under the terms of the
/// GNU Lesser General Public License as published by the Free Software Foundation; either version
/// 3.0 of the License, or (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
/// the GNULesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public License along with this
/// library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
/// Boston, MA 02110-1301 USA
///
/// @brief This file defines the functions of the AppleAVListEnumerator class.
///////////////////////////////////////////////////////////////////////////////////////////////////

#import <AVFoundation/AVFoundation.h>
#include "AppleAVListEnumerator.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Enumerate all A/V input devices, adding them to the base instance via AddVideoInput and
/// AddAudioInput.
///////////////////////////////////////////////////////////////////////////////////////////////////
void AppleAVListEnumerator::Enumerate()
{
	NSString* mediaType = AVMediaTypeVideo;
	NSArray* devices = [AVCaptureDevice devicesWithMediaType:mediaType];
	for (AVCaptureDevice *device in devices)
	{
    	std::string name([[device localizedName] UTF8String]);
    	AddVideoInput(name);
	}
	
	mediaType = AVMediaTypeAudio;
	devices = [AVCaptureDevice devicesWithMediaType:mediaType];
	for (AVCaptureDevice *device in devices)
	{
    	std::string name([[device localizedName] UTF8String]);
    	AddAudioInput(name);
	}
}
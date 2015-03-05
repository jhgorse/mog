///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file SenderPipeline_Ocpp.mm
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
/// @brief This file defines the Objective-C++ functions of the SenderPipeline class.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cstring>
#include <string>
#import <AVFoundation/AVFoundation.h>
#include "SenderPipeline.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::GetVideoDeviceIndex
///
/// Get the video device index for the given name.
///
/// @param inputName  The name of the video input.
///
/// @return  The integer device index, or -1 if the device name could not be found.
///////////////////////////////////////////////////////////////////////////////////////////////////
int SenderPipeline::GetVideoDeviceIndex(const char* inputName)
{
	std::string requestedName(inputName);
	NSString* mediaType = AVMediaTypeVideo;
	NSArray *devices = [AVCaptureDevice devicesWithMediaType:mediaType];
	for (AVCaptureDevice* device in devices)
	{
    	std::string name([[device localizedName] UTF8String]);
		if (name.compare(requestedName) == 0)
		{
			return [devices indexOfObject:device];
		}
	}
	return -1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::GetVideoDeviceCaps
///
/// Get the capabilities string for the given index.
///
/// @param inputIndex  The index of the video input.
///
/// @return  A NULL-terminated capabilities string (allocated with "new char[...]"), or NULL if the
/// given index could not be found. Should be freed with "delete[]".
///////////////////////////////////////////////////////////////////////////////////////////////////
const char* SenderPipeline::GetVideoDeviceCaps(int inputIndex)
{
	int width = 0, height = 0;
	
	NSString* mediaType = AVMediaTypeVideo;
	AVCaptureDevice* device = [[AVCaptureDevice devicesWithMediaType:mediaType] objectAtIndex:inputIndex];
	NSArray* formats = [device valueForKey:@"formats"];
	gdouble best_fps = 0.0;
	for (NSObject* f in [formats reverseObjectEnumerator])
	{
		CMFormatDescriptionRef formatDescription = (CMFormatDescriptionRef) [f performSelector:@selector(formatDescription)];
		CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);
		
		for (NSObject* rate in [f valueForKey:@"videoSupportedFrameRateRanges"])
		{
			gdouble max_fps;
			[[rate valueForKey:@"maxFrameRate"] getValue:&max_fps];
			if (((dimensions.width * dimensions.height) > (width * height)) && (max_fps >= 25.0))
			{
				width = dimensions.width;
				height = dimensions.height;
				best_fps = max_fps;
			}
		}
	}
	
	int fps_n = 0, fps_d = 0;
	gst_util_double_to_fraction(best_fps, &fps_n, &fps_d);
	char* result = new char[sizeof("video/x-raw, width=(int)-2147483648, height=(int)-2147483648, framerate=(fraction)-2147483648/-2147483648")];
	std::sprintf(result, "video/x-raw, width=(int)%d, height=(int)%d, framerate=(fraction)%d/%d", width, height, fps_n, fps_d);
	return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::GetAudioDeviceIndex
///
/// Get the audio device index for the given name.
///
/// @param inputName  The name of the audio input.
///
/// @return  The integer device index, or -1 if the device name could not be found.
///////////////////////////////////////////////////////////////////////////////////////////////////
int SenderPipeline::GetAudioDeviceIndex(const char* inputName)
{
	UInt32 inputNameLength = std::strlen(inputName);
	
	UInt32 propertySize = 0;
	AudioObjectPropertyAddress audioDevicesAddress = {
		kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	assert(AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &audioDevicesAddress, 0, NULL, &propertySize) == noErr);
	size_t numDevices = propertySize / sizeof(AudioDeviceID);
	AudioDeviceID* devices = new AudioDeviceID[numDevices];
	assert(devices != NULL);
    assert(AudioObjectGetPropertyData (kAudioObjectSystemObject, &audioDevicesAddress, 0, NULL, &propertySize, devices) == noErr);
    
    for (size_t i = 0; i < numDevices; ++i)
    {
    	AudioDeviceID deviceId = devices[i];
		AudioObjectPropertyAddress deviceNameAddress = {
			kAudioDevicePropertyDeviceName,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster
		};
		assert(AudioObjectGetPropertyDataSize(deviceId, &deviceNameAddress, 0, NULL, &propertySize) == noErr);
		char* deviceName = new char[propertySize];
		assert(AudioObjectGetPropertyData(deviceId, &deviceNameAddress, 0, NULL, &propertySize, deviceName) == noErr);
		
		if (std::strncmp(inputName, deviceName, std::min(inputNameLength, propertySize-1)) == 0)
		{
			delete[] deviceName;
			delete[] devices;
			return deviceId;
		}
		
		delete[] deviceName;
    }
    
	delete[] devices;
	
	return -1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::GetAudioDeviceCaps
///
/// Get the capabilities string for the given index.
///
/// @param inputIndex  The index of the audio input.
///
/// @return  A NULL-terminated capabilities string (allocated with "new char[...]"), or NULL if the
/// given index could not be found. Should be freed with "delete[]".
///////////////////////////////////////////////////////////////////////////////////////////////////
const char* SenderPipeline::GetAudioDeviceCaps(int inputIndex)
{
	char* result;
	
#if 0
// The following code *should* work to find the best audio format for the selected input.
// However, it doesn't seem to work, as we get various kinds of errors from the underlying
// audio source. So this is ifdef'ed out for now.
	AudioObjectPropertyAddress streamsAddress = {
		kAudioDevicePropertyStreams,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	UInt32 propertySize = 0;
	assert(AudioObjectGetPropertyDataSize(inputIndex, &streamsAddress, 0, NULL, &propertySize) == noErr);
	size_t numStreams = propertySize / sizeof(AudioStreamID);
	AudioStreamID* streams = new AudioStreamID[numStreams];
	assert(AudioObjectGetPropertyData(inputIndex, &streamsAddress, 0, NULL, &propertySize, streams) == noErr);
	
	int bits = 0;
	int rate = 0;
	int channels = 0;
	char endian = ' ';
	for (size_t i = 0; i < numStreams; ++i)
	{
		AudioObjectPropertyAddress formatsAddress = {
			kAudioStreamPropertyAvailablePhysicalFormats,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster
		};
		assert(AudioObjectGetPropertyDataSize(streams[i], &formatsAddress, 0, NULL, &propertySize) == noErr);
		size_t numFormats = propertySize / sizeof(AudioStreamRangedDescription);
		AudioStreamRangedDescription* formats = new AudioStreamRangedDescription[numFormats];
		assert(AudioObjectGetPropertyData(streams[i], &formatsAddress, 0, NULL, &propertySize, formats) == noErr);

		for (size_t j = 0; j < numFormats; ++j)
		{
			if (formats[j].mFormat.mFormatID == kAudioFormatLinearPCM)
			{
				if
				(
					(formats[j].mFormat.mBitsPerChannel * ((int)formats[j].mFormat.mSampleRate) * formats[j].mFormat.mChannelsPerFrame) > (bits * rate * channels)
					&& (formats[j].mFormat.mSampleRate <= 48000.0)
				)
				{
					bits = formats[j].mFormat.mBitsPerChannel;
					rate = (int)formats[j].mFormat.mSampleRate;
					channels = formats[j].mFormat.mChannelsPerFrame;
					endian = (((formats[j].mFormat.mFormatFlags & kCAFLinearPCMFormatFlagIsLittleEndian) != 0) ? 'L' : 'B');
				}
			}
		}
	
		delete[] formats;
	}
	
	delete[] streams;
	
	result = new char[sizeof("audio/x-raw, format=(string)S-2147483648LE, layout=(string)interleaved, rate=(int)-2147483648, channels=(int)-2147483648")];
	std::sprintf(result, "audio/x-raw, format=(string)S%d%cE, layout=(string)interleaved, rate=(int)%d, channels=(int)%d", bits, endian, rate, channels);
#else
	// Get device name
	AudioObjectPropertyAddress deviceNameAddress = {
		kAudioDevicePropertyDeviceName,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	UInt32 propertySize = 0;
	assert(AudioObjectGetPropertyDataSize(inputIndex, &deviceNameAddress, 0, NULL, &propertySize) == noErr);
	char* deviceName = new char[propertySize];
	assert(AudioObjectGetPropertyData(inputIndex, &deviceNameAddress, 0, NULL, &propertySize, deviceName) == noErr);

	result = new char[sizeof("audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)-2147483648, channels=(int)1")];
	if (std::strncmp(deviceName, "Built-in Mic", sizeof("Built-in Mic") - 1) == 0)
	{
		std::strcpy(result, "audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)44100, channels=(int)1");
	}
	else if (std::strncmp(deviceName, "Phnx MT202exe", sizeof("Phnx MT202exe") - 1) == 0)
	{
		std::strcpy(result, "audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)32000, channels=(int)1");
	}
	else
	{
		std::fprintf(stderr, "The input named \"%s\" is currently unsupported.\n", deviceName);
		assert(false);
	}
	delete[] deviceName;
#endif

	return result;
}
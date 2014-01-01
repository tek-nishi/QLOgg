
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <QuickLook/QuickLook.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Cocoa/Cocoa.h>
#include <array>
#include "co_streaming.hpp"


enum {
  MAX_DUMMY_LENGTH = 1024,
  MAX_PATH_LENGTH = 1024,
};


/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

extern "C"
OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options) {
  NSDictionary* dict = (__bridge NSDictionary*)options;

  NSLOG(@"QLPreviewOptions:\n%@", dict);

  NSNumber* previewMode = (NSNumber*)[dict objectForKey:@"QLPreviewMode"];
  if ([previewMode longValue] != 5) {
    // spcebarで起動しなかった
    return noErr;
  }
  
  // ダミーデータでプレビューをごまかす
  std::array<UInt8, MAX_DUMMY_LENGTH> dummy;
  dummy.fill(0);
  CFDataRef data = CFDataCreate(kCFAllocatorDefault, &dummy[0], dummy.size());
  QLPreviewRequestSetDataRepresentation(preview, data, kUTTypeAudio, nil);
  CFRelease(data);

  // CFURL→std::string
  std::array<char, MAX_PATH_LENGTH> file;
  CFURLGetFileSystemRepresentation(url, false, (UInt8*)&file[0], file.size());
  std::string path(&file[0]);
    
  // ogg vorbisストリーミング
  ngs::Streaming stream(path);
  stream.doStreaming(preview);

  return noErr;
}

extern "C"
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview) {
  // Implement only if supported
}

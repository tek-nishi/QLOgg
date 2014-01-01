
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <QuickLook/QuickLook.h>


/* -----------------------------------------------------------------------------
   Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

extern "C"
OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize) {
  // プラグインのバンドルから画像ファイルを取り出す
  CFBundleRef       bundleRef   = QLThumbnailRequestGetGeneratorBundle(thumbnail);
  CFURLRef          imageURLRef = CFBundleCopyResourceURL(bundleRef, CFSTR("icon"), CFSTR("png"), NULL);
  CGDataProviderRef provider    = CGDataProviderCreateWithURL(imageURLRef);
  CFRelease(imageURLRef);

  // 画像からCGImageを生成
  CGImageRef image = CGImageCreateWithPNGDataProvider(provider, NULL, false, kCGRenderingIntentDefault);
  CGDataProviderRelease(provider);

  // サムネイル画像として渡す
  QLThumbnailRequestSetImage(thumbnail, image, nil);
  CGImageRelease(image);
    
  return noErr;
}

extern "C"
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail) {
  // Implement only if supported
}

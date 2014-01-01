
#pragma once

//
// AudioQueueによるストリーミング
//

#include <AudioToolbox/AudioToolbox.h>
#include <QuickLook/QuickLook.h>
#include <Cocoa/Cocoa.h>
#include "co_defines.hpp"
#include "co_streamOgg.hpp"


namespace ngs {

class Streaming {
  enum { MAX_BUFFERSIZE = 0x10000 };

  StreamOgg ogg_;

  bool isRunning_;
  
  AudioStreamBasicDescription description_;
  AudioQueueRef               audioQueue_;
  AudioQueueBufferRef         audioQueueBuffers_[3];


public:
  explicit Streaming(const std::string& file) :
    ogg_(file),
    isRunning_(false)
  {
    // ストリーミング用バッファの用意
    FillOutASBDForLPCM(description_, ogg_.sampleRate(), ogg_.channels(), 16, 16, false, false);

    OSStatus status = AudioQueueNewOutput(&description_,
                                          outputCallback,
                                          this,
                                          CFRunLoopGetCurrent(),
                                          kCFRunLoopCommonModes,
                                          0, //reserved, must be 0 according to docs
                                          &audioQueue_);
    
    if(status != noErr) {
      NSLOG(@"Error AudioQueueNewOutput:%@", statusToString(status));
      exit(EXIT_FAILURE);
    }

    // バッファへ最初のデータを読み込んでおく
    isRunning_ = true;
    for (u_int i = 0; i < 3; i++) {
      OSStatus status = AudioQueueAllocateBuffer(audioQueue_,
                                                 MAX_BUFFERSIZE,
                                                 &audioQueueBuffers_[i]);
      if (status != noErr) {
        AudioQueueDispose(audioQueue_, true);
        audioQueue_ = 0;
        NSLOG(@"Error allocating audio queue buffer %d:%@", i, statusToString(status));
        exit(EXIT_FAILURE);
      }
      outputCallback(this, audioQueue_, audioQueueBuffers_[i]);
    }
    
    AudioQueueSetParameter(audioQueue_, kAudioQueueParam_Volume, 1.0);
  }

  ~Streaming() {
    AudioQueueDispose(audioQueue_, true);
  }
  

  // 再生(再生完了までブロックする)
  void doStreaming(QLPreviewRequestRef preview) {
    AudioQueueStart(audioQueue_, nullptr);

    while (isRunning_) {
      if (QLPreviewRequestIsCancelled(preview)) {
        NSLOG(@"QLPreviewRequestIsCancelled");
        stopPlayback();
        return;
      }
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
    }
    NSLOG(@"CFRunLoopRunInMode finished");

    //run a bit longer to fully flush audio buffers
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false);
  }

  void stopPlayback() {
    NSLOG(@"stopPlayback");
    isRunning_ = false;
    AudioQueueStop(audioQueue_, true);
  }


  // TIPS:同じクラスなので、privateな変数にアクセスできる
  static void outputCallback(void* aqData, AudioQueueRef inAQ, AudioQueueBufferRef pcmOut) {
   Streaming* stream = static_cast<Streaming*>(aqData);

   if (!stream->isRunning_) return;

    size_t read_size = stream->ogg_.read((char*)pcmOut->mAudioData, pcmOut->mAudioDataBytesCapacity);
    NSLOG(@"stream read:%ld", read_size);
  
    if (read_size == 0) {
      //we've reached EOF, let buffers drain before stopping
      NSLOG(@"outputCallback stop");
      AudioQueueStop(stream->audioQueue_, false);
      stream->isRunning_ = false;
      return;
    }
  
    pcmOut->mAudioDataByteSize = static_cast<UInt32>(read_size);
    pcmOut->mPacketDescriptionCount = 0;
    OSStatus status = AudioQueueEnqueueBuffer(stream->audioQueue_, pcmOut, 0, nullptr);
  
    if (status != noErr) {
      //something has gone horribly wrong, stop now
      AudioQueueStop(stream->audioQueue_, true);
      stream->isRunning_ = false;
      NSLOG(@"Error AudioQueueEnqueueBuffer:%@", statusToString(status));
      exit(EXIT_FAILURE);
    }
  }

  // エラーコードを文字列に変換
  static NSString* statusToString(const OSStatus status) {
    NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
    return [error description];
  }
  
};

}

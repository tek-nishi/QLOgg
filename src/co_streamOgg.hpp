
#pragma once

//
// Ogg Vorbisでのストリーミング
// TODO:エラー処理
//

#include "co_defines.hpp"
#include <iostream>
#include <vector>
#include <vorbis/vorbisfile.h>


namespace ngs {

class StreamOgg {
  OggVorbis_File fstr_;

  u_int ch_;
  long sample_rate_;
  u_int size_;

  size_t last_size_;

  bool loop_;

  enum { BIT_NUM = 2 };

public:
  explicit StreamOgg(const std::string& file) :
    loop_(false)
  {
    DOUT << "StreamOgg()" << std::endl;

    ov_fopen(file.c_str(), &fstr_);

    vorbis_info* info = ov_info(&fstr_, -1);
    ch_ = info->channels;
    sample_rate_ = info->rate;
    size_ = ch_ * BIT_NUM * static_cast<u_int>(ov_pcm_total(&fstr_, -1));
    last_size_ = size_;
    // DOUT << ch_ << " " << sample_rate_ << " " << last_size_ << std::endl;
  }

  ~StreamOgg() {
    DOUT << "~StreamOgg()" << std::endl;
    ov_clear(&fstr_);
  }


  bool isStereo() const { return ch_ == 2; }
  u_int channels() const { return ch_; }
  long sampleRate() const { return sample_rate_; }

  void loop(const bool loop) {
    loop_ = loop;
  }

  // 再生位置を先頭に戻す
  void toTop() {
    ov_time_seek(&fstr_, 0.0);
    last_size_ = size_;
  }

  bool isEnd() const { return last_size_ == 0; }

  // 再生バッファにデータを読み込む
  long read(char* buffer, const size_t size) {
    long remain_size = size;
    if (!loop_ && (last_size_ < remain_size)) remain_size = last_size_;
    // ループしない場合、残りの中途半端なサイズを読み込んで終了

    size_t offset = 0;
    size_t total_read_size = 0;
    int bitstream = 0;

    // バッファを満たすまでデータを読み込む(ループ再生の場合はさらに先頭に戻して読み込み)
    while (remain_size > 0) {
      long read_size = ov_read(&fstr_, &buffer[offset], static_cast<int>(remain_size), 0, BIT_NUM, 1, &bitstream);
      
      if (read_size <= 0) {
        DOUT << "read_size" << std::endl;
        return 0;
      }
      
      total_read_size += read_size;
      remain_size -= read_size;
      last_size_ -= read_size;
      offset += read_size;
      if (loop_ && !last_size_) toTop();
    }
    return total_read_size;
  }
  
};

}

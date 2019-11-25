// Copyright (C) 2019 Linkworld Open Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https: //www.gnu.org/licenses/>.
#include <arpa/inet.h>
#include "./frame.h"
using uint = unsigned int;
enum Type {
  FRS = FrameType::kResetStream,
  FSDB = FrameType::kStreamDataBlocked,
  FS = FrameType::kStream
};
// 长度信息标志
enum Flags { OEB = 0x00, TWB = 0x01, FOB = 0x02, EIB = 0x03, NON = 0xff };
//  将一个vint 类型写道相应一个数组中，第四个参数为从*location的位置开始写
//  第三个参数指明这一次写多长的数据
inline void WriteVintToBin(const vint &src_data, char *bin_data, int len,
                           int *location);
//  根据一个数据的长度返回标志，并且将标志位设置到相应的位置
inline uint GetLenAndSetFlag(vint *num);
void AppendStr(const char *const src_str, char *dir_str, int len, int begin);

//  FrameStream 转换到 GenericFrameLayout
enum StreamFlags { OFF = 0x04, LEN = 0x02, FIN = 0x01, SAV = 0x08 };
int FSToGFL(const FrameStream *const fs, GenericFrameLayout *gfl) {
  int id_len = 0, off_len = 0, len_len = 0, dat_len = 0, total_len = 0;
  vint copy_id = fs->id, copy_offset = 0, copy_len = 0;
  id_len = GetLenAndSetFlag(&copy_id);
  if (fs->bits & StreamFlags::OFF) {
    copy_offset = fs->offset;
    off_len = GetLenAndSetFlag(&(copy_offset));
  }
  if (fs->bits & StreamFlags::LEN) {
    copy_len = dat_len = fs->length;
    len_len = GetLenAndSetFlag(&copy_len);
  }
  total_len = id_len + off_len + len_len + dat_len + 1;
  char *total_data = new char[total_len]{0};
  if (total_data == nullptr) return -1;
  gfl->frame_type = Type::FS;
  gfl->data_len = total_len;
  int pos = 0;
  WriteVintToBin(fs->bits, total_data, 1, &pos);
  WriteVintToBin(copy_id, total_data, id_len, &pos);
  if (off_len) WriteVintToBin(copy_offset, total_data, off_len, &pos);
  if (len_len) {
    WriteVintToBin(copy_len, total_data, len_len, &pos);
    AppendStr(fs->stream_data, total_data, dat_len, pos);
  }
  gfl->data = total_data;
  return 0;
}
//  Convert FrameStreamDataBlocked to GenericFrmaeLayout
int FSDBToGFL(const FrameStreamDataBlocked *const fsdb,
              GenericFrameLayout *gfl) {
  vint copy_id = fsdb->stream_id;
  vint copy_stream_data_limit = fsdb->stream_data_limit;
  int id_len = GetLenAndSetFlag(&copy_id);
  int sdl_len = GetLenAndSetFlag(&copy_stream_data_limit);
  int total_len = id_len + sdl_len;
  char *total_data = new char[total_len]{0};
  if (total_data == nullptr) return -1;
  gfl->data_len = total_len;
  gfl->frame_type = Type::FSDB;
  int pos = 0;
  WriteVintToBin(copy_id, total_data, id_len, &pos);
  WriteVintToBin(copy_stream_data_limit, total_data, sdl_len, &pos);

  gfl->data = total_data;
  return 0;
}
int FRSToGFL(const FrameResetStream *const fs, GenericFrameLayout *gfl) {
  vint copy_id = fs->stream_id;
  vint copy_error_code = fs->error_code;
  vint copy_final_size = fs->final_size;
  int id_len = GetLenAndSetFlag(&copy_id);
  int ec_len = GetLenAndSetFlag(&copy_error_code);
  int fz_len = GetLenAndSetFlag(&copy_final_size);
  int total_len = id_len + ec_len + fz_len;
  char *total_data = new char[total_len]{0};
  if (total_data == nullptr) return -1;
  gfl->frame_type = Type::FRS;
  gfl->data_len = total_len;
  int pos = 0;
  WriteVintToBin(copy_id, total_data, id_len, &pos);
  WriteVintToBin(copy_error_code, total_data, ec_len, &pos);
  WriteVintToBin(copy_final_size, total_data, fz_len, &(pos));

  gfl->data = total_data;
  return 0;
}
//  成功转换返回0 失败返回-1
int ConvertFrameToGFL(const void *const frame, const FrameType Frame_type,
                      GenericFrameLayout *gfl) {
  switch (Frame_type) {
    case FrameType::kStream:
      return FSToGFL(reinterpret_cast<const FrameStream *const>((frame)), gfl);
    case FrameType::kStreamDataBlocked:
      return FSDBToGFL(
          reinterpret_cast<const FrameStreamDataBlocked *const>((frame)), gfl);
    case FrameType::kResetStream:
      return FRSToGFL(reinterpret_cast<const FrameResetStream *const>((frame)),
                      gfl);
    default:
      return -1;
  }
  return 0;
}
//  获取某一个类型的大小 并且为这个类型设置flag
//  如果传入一个负数，或者超出这个范围那么将返回0
inline uint GetLenAndSetFlag(vint *num) {
  if ((*num >= 0 && *num <= 63)) {
    *(num) |= Flags::OEB << 6;
    return 1;
  } else if (*num > 63 && *num <= 16383) {
    *(num) |= Flags::TWB << 14;
    return 2;
  } else if ((*num > 16383 && *num <= 1073741823)) {
    *(num) |= Flags::FOB << 30;
    return 4;
  } else if (*num > 1073741823 && *num <= 4611686018427387903) {
    *(num) |= (vint)(Flags::EIB) << 62;
    return 8;
  }
  return 0;
}
//  传入的这个值需要设置好flag 从数组头部开始写值
inline void WriteVintToBin(const vint &src_data, char *bin_data, int len,
                           int *location) {
  for (int i = 0, j = len - 1; i < len; ++i, ++*(location), --j) {
    *(bin_data + *(location)) |= (((src_data) >> ((j)*8)) & 0xff);
  }
}
//  Append string to dir_str in begin
inline void AppendStr(const char *const src_str, char *dir_str, int len,
                      int begin) {
  for (int i = 0; i < len; ++i, ++begin) {
    *(dir_str + begin) = *(src_str + i);
  }
}
//  删除vint中的标志并且返回这个标志代表的长度
inline void RemoveFlags(vint *vint_data, const uint &flag) {
  switch (flag) {
    case Flags::OEB:
      break;
    case Flags::TWB:
      *vint_data &= 0x3fff;
      break;
    case Flags::FOB:
      *vint_data &= 0x3fffffff;
      break;
    case Flags::EIB:
      *vint_data &= 0x3fffffffffffffff;
      break;
  }
}
inline int FlagToLen(const uint &flag) {
  switch (flag) {
    case Flags::OEB:
      return 1;
    case Flags::TWB:
      return 2;
    case Flags::FOB:
      return 4;
    case Flags::EIB:
      return 8;
    default:
      return 0;
  }
  return 0;
}
//  从二进制数据中写入*vint数组中  这个函数针对FSDB FRS类型的对象
inline void WriteBinToVint(const char *const src_data, vint **dest_data,
                           uint num) {
  //  一个参数为二进制数据对象，
  //  第二参数个是目标数组,
  //  第三个参数指明第二个参数拥有几个参数
  int location = 0;
  for (uint i = 0; i < num; ++i) {
    uint8_t tmpFlag = (*(src_data + location) & 0xC0) >> 6;
    int len = FlagToLen(tmpFlag);
    int begin = location;
    location += len;  //  此时location指向下一个标志所在的空间
    while (len-- > 0) {
      *(*(dest_data + i)) |= ((*(src_data + begin) & 0xff) << (len * 8));
      ++begin;
    }
    RemoveFlags(*(dest_data + i), tmpFlag);
  }
}
int GFLToFRS(const GenericFrameLayout *const gfl, FrameResetStream *frame) {
  vint *vint_array[3] = {&(frame->stream_id = 0), &(frame->error_code = 0),
                         &(frame->final_size = 0)};
  WriteBinToVint(gfl->data, vint_array, 3);
  return 0;
}
int GFLToFSDB(const GenericFrameLayout *const gfl,
              FrameStreamDataBlocked *frame) {
  vint *vint_array[2] = {&(frame->stream_id = 0),
                         &(frame->stream_data_limit = 0)};
  WriteBinToVint(gfl->data, vint_array, 2);
  return 0;
}
int GFLToFS(const GenericFrameLayout *const gfl, FrameStream *frame) {
  int location = 1;  // 数据开始的位置
  frame->stream_data = nullptr;
  // 若没有数据存放的时候不设置为Nullptr那在程序结束的时候会释放一个无效指针
  auto readF = [&](vint *dest_data) mutable {
    uint8_t tmpFlag = (*(gfl->data + location) & 0xC0) >> 6;
    int len = FlagToLen(tmpFlag);
    int begin = location;
    location += len;  //  此时location指向下一个标志所在的空间
    while (len-- > 0) {
      *(dest_data) |= ((*(gfl->data + begin) & 0xff) << len * 8);
      ++begin;
    }
    RemoveFlags(dest_data, tmpFlag);
  };
  readF(&(frame->id = 0));
  frame->bits = *(gfl->data + 0);
  if (frame->bits & StreamFlags::OFF) {
    readF(&(frame->offset = 0));
  }
  if (frame->bits & StreamFlags::LEN) {
    readF(&(frame->length = 0));
    frame->stream_data = new char[frame->length]{0};
    for (uint i = 0; i < frame->length; ++i) {
      (frame->stream_data)[i] = *(gfl->data + location + i);
    }
  }
  return 0;
}
int ConvertGFLToFrame(const GenericFrameLayout *const gfl, void *frame,
                      FrameType *frameType) {
  switch (gfl->frame_type) {
    case Type::FRS:
      *frameType = FrameType::kResetStream;
      return GFLToFRS(gfl, reinterpret_cast<FrameResetStream *>(frame));
    case Type::FSDB:
      *frameType = FrameType::kStreamDataBlocked;
      return GFLToFSDB(gfl, reinterpret_cast<FrameStreamDataBlocked *>(frame));
    case Type::FS:
      *frameType = FrameType::kStream;
      return GFLToFS(gfl, reinterpret_cast<FrameStream *>(frame));
    default:
      return -1;
  }
}

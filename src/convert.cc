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
enum Type { FRS = 0x04, FSDB = 0xF };
// 长度信息标志
enum Flags { OEB = 0x00, TWB = 0x01, FOB = 0x02, EIB = 0x03, NON = 0xff };
//  将一个vint 类型写道相应一个数组中，第四个参数为从*location的位置开始写
//  第三个参数指明这一次写多长的数据
void WriteVintToBin(const vint &src_data, char *bin_data, uint len,
                    uint *location);
//  根据一个数据的长度返回标志，并且将标志位设置到相应的位置
uint GetLenAndSetFlag(vint *num);

//  FrameStream 转换到 GenericFrameLayout
enum { OFF = 0x04, LEN = 0x02, FIN = 0x01, SAV = 0x08 };
int FSToGFL(const FrameStream *const fs, GenericFrameLayout *gfl) {
  int id_len = 0, off_len = 0, len_len = 0, dat_len = 0, total_len = 0;
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
  WriteVintToBin(copy_stream_data_limit, total_data, id_len, &(--total_len));
  WriteVintToBin(copy_id, total_data, id_len, &total_len);

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
  WriteVintToBin(copy_final_size, total_data, fz_len, &(--location));
  WriteVintToBin(copy_error_code, total_data, ec_len, &location);
  WriteVintToBin(copy_id, total_data, id_len, &location);
}
int ConvertFrameToGFL(const void *const frame, const FrameType Frame_type,
                      GenericFrameLayout *gfl) {
  switch (Frame_type) {
    case FrameType::kStream:
      return FSToGFL(reinterpret_cast<FrameStream *>(const_cast<void *>(frame)),
                     gfl);
    case FrameType::kStreamDataBlocked:
      return FSDBToGFL(
          reinterpret_cast<FrameStreamDataBlocked *>(const_cast<void *>(frame)),
          gfl);
    case FrameType::kResetStream:
      return FRSToGFL(
          reinterpret_cast<FrameResetStream *>(const_cast<void *>(frame)), gfl);
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
    *(num) |= Flags::EIB << 62;
    return 8;
  }
  return 0;
}
//  传入的这个值需要设置好flag 从数组的尾部开始写值
void WriteVintToBin(const vint &src_data, char *bin_data, int len,
                    int *location) {
  uint8_t tmpData = 0;
  while (len-- >= 0) {
    tmpData = 0;  // clean up
    tmpData |= (((src_data) >> ((len)*8)) & 0xff);
    *(bin_data + *location) |= tmpData;
    --*(location);
  }
}

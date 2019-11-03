/*
Copyright 2019 YHTB
*/
#include "./frame.h"
using uint = unsigned int;
enum { Byte = 8, VintTotalBit = 64 };
//----------------------------------------------------------------------------//
// 各种对象到二进制的转换
enum Type { FRS = 0x04, FSDB = 0x15 };

// 获取64位无符号整形的第一二位
inline uint8_t GetFlag(const vint num, const uint bits = 62) {
  uint8_t tmp = num >> bits;
  return reinterpret_cast<uint8_t>(tmp);
}
inline uint8_t GetFlag(const char *const bin, const uint bits = 62) {
  uint8_t tmp = *bin >> bits;
  return reinterpret_cast<uint8_t>(tmp);
}
//获取一个字段的长度 单位是byte
inline uint GetLen(uint flag) {
  switch (flag) {
    case 0x0:
      return 1;
    case 0x1:
      return 2;
    case 0x2:
      return 4;
    case 0x3:
      return 8;
    default:
      return 0;
  }
}
// vint<=>bin
char *vintTobin(const vint num) {
  char *tmp;
  switch (GetFlag(num)) {
    case 0x0:
      tmp = new char[1]{0};
      break;
    case 0x1:
      tmp = new char[2]{0};
      break;
    case 0x2:
      tmp = new char[4]{0};
      break;
    case 0x3:
      tmp = new char[8]{0};
      break;
    default:
      return nullptr;
  }
  *tmp |= num;
  return tmp;
}
// 合并数据,将orginData 的数据合并到location 第四个参数的单位为byte
void MergeData(const char OrginData[], char DirData[], uint *location,
               uint len) {
  for (uint i = 0; i < len; ++i) {
    DirData[(*location)++] = OrginData[i];
  }
}
struct tmpInfo {
  uint len;    //储存长度
  char *data;  //用于存放数据
};
// 接受一个tmpinfo的数组，第二个参数说明需要打包几个元素，第三个参数指明数据存放到哪里
// 为解决如何高效率的包装包
void MergeData(const tmpInfo *const pack, uint num, char *dirData) {
  uint location = 0;
  for (uint i = 0; i < num; ++i) {
    MergeData((*(pack + i)).data, dirData, &location, (*(pack + i)).len);
  }
}
// Frs => GFL
void FrsToGFL(FrameResetStream *Data, GenericFrameLayout *result) {
  // tmpInfo *data = new tmpInfo[3];
  result->frame_type = Type::FRS;
  //其中每一个字段的长度以及总长度
  uint Id_len = GetLen(GetFlag(Data->stream_id));
  uint Er_len = GetLen(GetFlag(Data->error_code));
  uint Fi_len = GetLen(GetFlag(Data->final_size));
  uint total_len = Id_len + Er_len + Fi_len;
  // 为总数据申请内存；
  char *total_data = new char[total_len]{0};
  // 将每一段的数据都放total_data 每一次存放一个字节
  uint location = 0;  // 记录每一次存放后的数据在总数据中的地址位置
  // 暂用方案，以后可能采取打包成数组的方式
  char *id_data = vintTobin(Data->stream_id);
  char *err_data = vintTobin(Data->error_code);
  char *final_data = vintTobin(Data->final_size);
  MergeData(id_data, total_data, &location, Id_len);
  MergeData(err_data, total_data, &location, Id_len);
  MergeData(final_data, total_data, &location, Id_len);

  result->data = total_data;
  //释放占用的堆资源
  delete[] id_data;
  delete[] err_data;
  delete[] final_data;
}
// FSDB => GFL 这个类型对象的方法同上 注释略
void FsdbToGFL(FrameStreamDataBlocked *data, GenericFrameLayout *result) {
  result->frame_type = Type::FSDB;
  // 获取长度
  uint Id_len = GetLen(GetFlag(data->stream_id));
  uint Sdl_len = GetLen(GetFlag(data->stream_data_limit));
  uint total_len = Id_len + Sdl_len;

  char *total_data = new char[total_len]{0};
  uint location = 0;
  char *id_data = vintTobin(data->stream_id);
  char *SDL_data = vintTobin(data->stream_data_limit);
  MergeData(id_data, total_data, &location, Id_len);
  MergeData(SDL_data, total_data, &location, Sdl_len);
  result->data = total_data;
  delete[] id_data;
  delete[] SDL_data;
}
// FS => GFL
// 用于判fs类型中是否有某些字段，若没有将不给相应的位段分配内存空间
enum Mask { OFF = 0x04, LEN = 0x02, FIN = 0x01, SAV = 0x08 };
void FsToGFL(FrameStream *data, GenericFrameLayout *result) {
  result->frame_type = data->bits;
  // offest length长度的字段
  uint off_len = 0, len_len = 0, id_len = 0, DataLen = 0;
  uint location = 0;  // 存放稍后合并数据时候的实时位置
  // 申请ID 的储存空间
  id_len = GetLen(GetFlag(data->id));
  char *id_data = vintTobin(data->id);
  // 检查是否有offest 字段，若有则位offest申请空间
  if (((data->bits) & Mask::OFF) != 0) {
    off_len = GetLen(GetFlag(data->offset));
  }
  if (((data->bits) & Mask::LEN) != 0) {
    len_len = GetLen(GetFlag(data->length));
    DataLen = data->length;
  }
  // 为总数居申请空间
  char *total_data = new char[off_len + len_len + DataLen];
  MergeData(id_data, total_data, &location, id_len);
  delete[] id_data;
  if (off_len) {
    char *off_data = vintTobin(data->offset);
    MergeData(off_data, total_data, &location, off_len);
    delete[] off_data;
  }
  if (len_len) {
    char *len_data = vintTobin(data->length);
    MergeData(len_data, total_data, &location, len_len);

    // 若len_不为零 就要将函数中的stream_data合并进去；
    MergeData(data->stream_data, total_data, &location, DataLen);
    delete[] len_data;
  }
}
int ConvertFrameToGFL(const void *const frame, const FrameType Frame_type,
                      GenericFrameLayout *gfl) {
  switch (Frame_type) {
    case FrameType::kStream:
      FsToGFL((FrameStream *)(frame), gfl);
      break;
    case FrameType::kStreamDataBlocked:
      FsdbToGFL((FrameStreamDataBlocked *)frame, gfl);
      break;
    case FrameType::kResetStream:
      FrsToGFL((FrameResetStream *)frame, gfl);
      break;
    default:
      return -1;
  }
  return Frame_type;
}

//----------------------------------------------------------------------
// convert bin to struct
// 将数据从二进制中提取出来

//从其中的第一个字符 并分析这个字段的长度 FrameStream单独一种方式
uint Getlen(const char *const orgin) {
  char tmpC = *(orgin + 0);  // 将字符串的值第一个值提取出来并且当
}
int ExtractData(const char *const orgin, vint *data) {}
// GFL to FSDB
void GflToFsdb(const GenericFrameLayout *orgin, FrameStreamDataBlocked *dir) {}
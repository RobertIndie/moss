/*
Copyright 2019 Linkworld Open Team
                              YHTB
*/
#include <iostream>
#include <limits>
#include "./frame.h"
using uint = unsigned int;
enum { Byte = 8, VintTotalBit = 64 };
//----------------------------------------------------------------------------//
// 各种对象到二进制的转换
enum Type { FRS = 0x04, FSDB = 0x15 };

//---------------------------------------------------------------------------------
// 为数据添加标志的相关函数
/*
Copyright YHTB 2019
AddFlag Template Function
*/
#include "./frame.h"
using uint = unsigned int;
// 编译期常量
template <typename T>
constexpr uint64_t TypeMax = std::numeric_limits<T>::max();
template <typename T>
constexpr uint64_t TypeMin = std::numeric_limits<T>::min();
// 返回一个类型的大小
template <typename T>
constexpr uint TypeSize = sizeof(T);
// 返回一个整形是否在T的范围中
template <typename T>
inline bool Rang(uint64_t num) {
  return ((num >= TypeMin<T> && num <= TypeMax<T>) ? true : false);
}
// 获取num所需的最小空间
uint GetMinLen(vint *num) {
  // 由小到大的检查给定的数值是在哪一个类型之间的，当找到的时候返回这个字节的数值
  if (Rang<uint8_t>(*num)) {
    return TypeSize<uint8_t>;
  } else if (Rang<uint16_t>(*num)) {
    return TypeSize<uint16_t>;
  } else if (Rang<uint32_t>(*num)) {
    return TypeSize<uint32_t>;
  } else if (Rang<uint64_t>(*num)) {
    return TypeSize<uint64_t>;
  }
  return 0;
}
// 获得标志位 这些标志位放在小端 使用时候需要根据需求来向左移相应的位数
// 若没有匹配到相应的字节数那么返回0xff
uint8_t GetFlag(uint DataLen) {
  switch (DataLen) {
    case 1:
      return 0x00;
    case 2:
      return 0x01;
    case 4:
      return 0x02;
    case 8:
      return 0x03;
    default:
      return 0xff;
  }
}
// 跟据字节添加标志
void SetFlag(vint *data) {
  uint MinAir = GetMinLen(data);
  vint tmpFlag = GetFlag(MinAir);
  if (tmpFlag == 0xff) {
    std::cout << "DataFiled" << std::endl;
    return;  // 终止程序
  }
  tmpFlag <<= 62;  // 将标志位移动到应该设置的位置
  *data |= tmpFlag;
}

//-------------------------------------------------------------------------------

// 获取64位无符号整形的第一二位
inline uint8_t GetFlag(vint num, const uint bits = 62) {
  uint8_t tmp = num >> bits;
  // 删掉标志位
  return reinterpret_cast<uint8_t>(tmp);
}
// 删除vint中的标志，无论这个数据是否存在标志位 ，都会执行语句，不做任何的检查
inline void RemoveFlag(vint *data) { (*(data) <<= 2) >>= 2; }
// 将一个vint
// 类型的存放到相应的数组中，第一个原数据，第二个参数是目标地址，第三个参数指向对象需要的字节数
// 注意：会出现的标志位被占用的情况
void AccessVintToArray(const vint data, char *array, uint len) {
  enum { Mask = 0xff };
  uint totalBit = len * 8;
  char tmpPartData;  //言数据中的
  for (uint i = 0; i < len; ++i) {
    // 将一个字节的数据从中提取出来
    tmpPartData = (data >> (totalBit - (i + 1) * 8)) & Mask;
    *(array + i) |= tmpPartData;
  }
}
// 将一个数组从转换到一个大整形中
// 第一个参数为储存数据的数组，第二个参数是指明第一个参数数组的长度，第三个参数数据的目标位置
void AccessArrayToVint(const char *const SrcData, const uint ArrayLen,
                       vint *DirData) {
  enum { Mask = 0xff };
  vint tmpByteData = 0;  // 临时储存一字节的数据，并且将其放到适合的位置
  for (uint i = ArrayLen - 1, shiftBit = 0; i > 0; --i, ++shiftBit) {
    tmpByteData = 0;  // cleanUp
    tmpByteData = *(SrcData + i) & Mask;
    tmpByteData <<= shiftBit * 8;
    *DirData |= tmpByteData;
  }
}
//  获取一个字段的长度 单位是byte
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
// 申请完控制之后将字符数组的前两位添加flag
char *vintTobin(vint num) {
  char *tmp;
  uint len = 0;
  switch (GetFlag(num)) {
    case 0x00:
      tmp = new char[1]{0};
      len = 1;
      break;
    case 0x01:
      tmp = new char[2]{0};
      len = 2;
      break;
    case 0x02:
      tmp = new char[4]{0};
      len = 4;
      break;
    case 0x03:
      tmp = new char[8]{0};
      len = 8;
      break;
    default:
      return nullptr;
  }
  if (tmp != nullptr) {
    AccessVintToArray(num, tmp, len);  // had done
  }
  return tmp;
}
// 合并数据,将orginData 的数据合并到location 第四个参数的单位为byte
void MergeData(const char OrginData[], char dest_data[], uint *location,
               uint len) {
  for (uint i = 0; i < len; ++i) {
    dest_data[(*location)++] = OrginData[i];
  }
}
struct TmpInfo {
  uint len;    //储存长度
  char *data;  //用于存放数据
};
// 接受一个tmpinfo的数组，第二个参数说明需要打包几个元素，第三个参数指明数据存放到哪里
// 为解决如何高效率的包装包
void MergeData(const TmpInfo *const pack, uint num, char *dirData) {
  uint location = 0;
  for (uint i = 0; i < num; ++i) {
    MergeData((*(pack + i)).data, dirData, &location, (*(pack + i)).len);
  }
}
// Frs => GFL
void FRSToGFL(FrameResetStream *Data, GenericFrameLayout *result) {
  // tmpInfo *data = new tmpInfo[3];
  result->frame_type = Type::FRS;
  //其中每一个字段的长度以及总长度
  SetFlag(&(Data->stream_id));
  SetFlag(&(Data->error_code));
  SetFlag(&(Data->final_size));
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
  RemoveFlag(&(Data->stream_id));
  RemoveFlag(&(Data->error_code));
  RemoveFlag(&(Data->final_size));

  result->data = total_data;
  //释放占用的堆资源
  delete[] id_data;
  delete[] err_data;
  delete[] final_data;
}
// FSDB => GFL 这个类型对象的方法同上 注释略
void FSDBToGFL(FrameStreamDataBlocked *data, GenericFrameLayout *result) {
  result->frame_type = Type::FSDB;
  // 获取长度
  SetFlag(&(data->stream_id));
  SetFlag(&(data->stream_data_limit));
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
  RemoveFlag(&(data->stream_data_limit));
  RemoveFlag(&(data->stream_id));
  delete[] id_data;
  delete[] SDL_data;
}
// FS => GFL
// 用于判fs类型中是否有某些字段，若没有将不给相应的位段分配内存空间
enum Mask { OFF = 0x04, LEN = 0x02, FIN = 0x01, SAV = 0x08 };
void FSToGFL(FrameStream *data, GenericFrameLayout *result) {
  result->frame_type = data->bits;
  // offest length长度的字段
  uint off_len = 0, len_len = 0, id_len = 0, DataLen = 0;
  uint location = 0;  // 存放稍后合并数据时候的实时位置
  // 申请ID 的储存空间
  SetFlag(&(data->id));
  id_len = GetLen(GetFlag(data->id));
  char *id_data = vintTobin(data->id);
  // 检查是否有offest 字段，若有则位offest申请空间
  if (((data->bits) & Mask::OFF) != 0) {
    SetFlag(&(data->offset));
    off_len = GetLen(GetFlag(data->offset));
  }
  if (((data->bits) & Mask::LEN) != 0) {
    SetFlag(&(data->length));
    len_len = GetLen(GetFlag(data->length));
    DataLen = data->length;
  }
  // 为总数居申请空间
  char *total_data = new char[off_len + len_len + DataLen];
  MergeData(id_data, total_data, &location, id_len);
  RemoveFlag(&(data->id));
  delete[] id_data;
  if (off_len) {
    char *off_data = vintTobin(data->offset);
    MergeData(off_data, total_data, &location, off_len);
    delete[] off_data;
    RemoveFlag(&(data->length));
  }
  if (len_len) {
    char *len_data = vintTobin(data->length);
    MergeData(len_data, total_data, &location, len_len);

    // 若len_不为零 就要将函数中的stream_data合并进去；
    MergeData(data->stream_data, total_data, &location, DataLen);
    delete[] len_data;
    RemoveFlag(&(data->length));
  }
}
// 成功返回对象类型，失败返回-1；
int ConvertFrameToGFL(const void *const frame, const FrameType Frame_type,
                      GenericFrameLayout *gfl) {
  switch (Frame_type) {
    case FrameType::kStream:
      FSToGFL((FrameStream *)(frame), gfl);
      break;
    case FrameType::kStreamDataBlocked:
      FSDBToGFL((FrameStreamDataBlocked *)frame, gfl);
      break;
    case FrameType::kResetStream:
      FRSToGFL((FrameResetStream *)frame, gfl);
      break;
    default:
      return -1;
  }
  return Frame_type;
}

//----------------------------------------------------------------------
// convert bin to struct
// 将数据从二进制中提取出来

//从其中的第一个字符 并分析这个字段的长度
// 若location中的值 （条件）将试图取得下个位置信息可能存在的位置 并返回；
enum LenMask { LENG = 0xC0 };
uint GetLen(const char *const orgin, uint location = 0) {
  char LocationInfo = *(orgin + location);  // 将二进制的中的
  // 一个字节提取出来
  uint LenBits = LocationInfo & LenMask::LENG;  // 得到长度信息
  LenBits >>= 6;                   // 将标志位移动到末尾用于匹配
  uint LenByte = GetLen(LenBits);  // 将信息转换为长度字节信息
  // location += LenByte + 1;
  return LenByte;
}
// 提却数据，也就是二进制到 vint 的转化
// 第一个参数指明原始二进制数据，第二个参数为数据的目标存放地址（为一个数组），第三个参数指明第二个参数应该有几个元素
// 若在其中为分离字串的时候失败，那么返回0，若成功将返回1；
// 最后四个参数只是为了方便转换到FrameStream 的类型
int BinToVint(const char *const orgin, vint *data, uint num,
              bool fsFlag = false, bool fsFlag_off = false,
              bool fsFlag_len = false, char *stream_data = nullptr) {
  uint location = 0;  //字符串的初始位置
  if (fsFlag)
    num = 1;  // 若要转换的类型是FrameStream 那么将这个代码端只提取ID字段
  for (uint i = 0; i < num; ++i) {
    uint LenByte = GetLen(orgin, location);  // Get length Info;
    uint tmpBegin = location;  // 临时储存上一个信息位置，
    location += LenByte;  // 此时location 位置指向下一个长度信息的
    // 将其中的字符串提取出来
    char *tmpSubString = new char[LenByte];  // 储存子串
    if (tmpSubString == nullptr) return 0;
    for (uint i = 0; i < LenByte; ++i) {
      tmpSubString[i] = orgin[tmpBegin + i];
    }
    AccessArrayToVint(tmpSubString, LenByte, (data + i));
    // 释放临时变量
    delete[] tmpSubString;
  }
  // 为了FrameStream stream_data
  // 提取ID段的数据之后 此时location位置指向offest || length
  // 若fsFlag_off 被置一 那么代表有offest 字段
  if (fsFlag && fsFlag_off) {
    uint LenByte = GetLen(orgin, location);
    uint tmpBegin = location;
    location += tmpBegin;
    char *tmpSubString = new char[LenByte];  // 储存子串
    if (tmpSubString == nullptr) return 0;
    for (uint i = 0; i < LenByte; ++i) {
      tmpSubString[i] = orgin[tmpBegin + i];
    }
    AccessArrayToVint(tmpSubString, LenByte, (data + 1));  // 将数据存放到目的地
    // 释放临时变量
    delete[] tmpSubString;
  }
  if (fsFlag && fsFlag_len) {
    uint LenByte = GetLen(orgin, location);
    uint tmpBegin = location;
    location += tmpBegin;
    char *tmpSubString = new char[LenByte];  // 储存子串
    if (tmpSubString == nullptr) return 0;
    for (uint i = 0; i < LenByte; ++i) {
      tmpSubString[i] = orgin[tmpBegin + i];
    }
    //*(data + 1) |= *tmpSubString;  // 将数据存放到目的地
    AccessArrayToVint(tmpSubString, LenByte, (data + 1));
    // 释放临时变量
    delete[] tmpSubString;
    uint64_t length = (*(data + 1) << 2);  //消除高位的标志位；
    stream_data = new char[length];
    if (stream_data == nullptr) return 0;
    for (uint i = 0; i < length; ++i) {
      stream_data[i] = orgin[location + i];
    }
  }
  return 1;
}
// 确保用户输入的是有效数据，不对数据的有效性进行检查
// GFL to FSDB
// 返回提取结果，若其中有一个为假那么返回错误
int GFLToFSDB(const GenericFrameLayout *orgin, FrameStreamDataBlocked *dir) {
  vint *data = new vint[2];  //用于储存从中提取的数据
  uint result = BinToVint(orgin->data, data, 2);
  if (result == 0) return 0;
  dir->stream_id = data[0];
  dir->stream_data_limit = data[1];
  return 1;
}
// GFL-> FRS
int GFLToFRS(const GenericFrameLayout *orgin, FrameResetStream *dir) {
  vint *data = new vint[3];  //用于储存从中提取的数据
  uint result = BinToVint(orgin->data, data, 3);
  if (result == 0) return 0;
  dir->stream_id = data[0];
  dir->error_code = data[1];
  dir->final_size = data[2];
  return 1;
}

// GFL -> FS
// 若转换失败则返回0
int GFLToFS(const GenericFrameLayout *orgin, FrameStream *dir) {
  dir->bits = orgin->frame_type;
  // 先分析这个类型，来判断总数居中有几个数据
  uint LenBit = 0, OffBit = 0;
  if ((orgin->frame_type & Mask::LEN) == Mask::LEN) ++LenBit;
  if ((orgin->frame_type & Mask::OFF) == Mask::OFF) ++OffBit;
  vint *data = new vint[1 + LenBit + OffBit];
  if (BinToVint(orgin->data, data, 3, true, OffBit, LenBit, dir->stream_data)) {
    return 0;
  }
  dir->id = *(data + 0);
  if (OffBit) {
    dir->offset = *(data + 1);
    if (LenBit)
      dir->length = *(data + 2);
    else
      dir->length = 0;
  } else {
    dir->offset = 0;
    if (LenBit)
      dir->length = *(data + 1);
    else
      dir->length = 0;
  }
  return 1;
}
inline bool CheckFS(vint bits) { return ((bits >= 0x08) && (bits <= 0x0f)); }
// 成功的话返回1 转换失败返回-1 有每一个转换函数引起的错误一般为分配内存失败
int ConvertGFLToFrame(const GenericFrameLayout *const gfl, void *frame,
                      FrameType *frameType) {
  switch (gfl->frame_type) {
    case Type::FRS:
      *frameType = FrameType::kResetStream;
      return GFLToFRS(gfl, reinterpret_cast<FrameResetStream *>(frame));
    case Type::FSDB:
      *frameType = FrameType::kStreamDataBlocked;
      return GFLToFSDB(gfl, reinterpret_cast<FrameStreamDataBlocked *>(frame));
    default:
      if (CheckFS(gfl->frame_type)) {
        *frameType = FrameType::kStream;
        return GFLToFS(gfl, reinterpret_cast<FrameStream *>(frame));
      }
      return -1;
  }
}

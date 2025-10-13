#ifndef _TYPES_H_
#define _TYPES_H_

typedef signed char int8_t;     //8位有符号整数
typedef unsigned char uint8_t;  //8位无符号整数
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

// typedef uint32_t size_t;  //无符号大小类型，用于表示对象大小和数组索引
// typedef int32_t ssize_t;  //有符号大小类型，可以表示错误值（通常为-1）

#define NULL ((void*)0)  //空指针常量

#endif // _TYPES_H_
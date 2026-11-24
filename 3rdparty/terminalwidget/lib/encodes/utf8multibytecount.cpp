// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utf8multibytecount.h"

/*!
 * @brief 计算 UTF-8 字符的字节数
 *
 *  如果一个字节的第一位是0，则这个字节单独就是一个字符；如果第一位是1，则连续有多少个1，就表示当前字符占用多少个字节。
 *  0000 0000-0000 007F | 0xxxxxxx
 *  0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *  0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *  0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * @link https://zh.wikipedia.org/wiki/UTF-8
 */
int utf8MultiByteCount(char *buf, size_t size)
{
    // UTF 字符类型，单字符，中间字符，双字节，三字节，四字节
    enum UtfCharType {
        Single,
        Mid,
        DoubleBytes,
        ThreeBytes,
        FourBytes,
    };

    // 取得 UTF-8 字节序数值
    auto LeftBitFunc = [](unsigned char data) -> int {
        // 返回前导1的个数
        int res = 0;
        while (data & 0x80) {
            res++;
            data <<= 1;
        }
        return res;
    };

    int count = 0;
    while (size > 0 && count < FourBytes) {
        int leftBits = LeftBitFunc(*buf);
        switch (leftBits) {
            case Mid:
                count++;
                break;
            case DoubleBytes:
            case ThreeBytes:
            case FourBytes:
                return leftBits;
            default:
                // 超过4字节或单字节，均返回1
                return 1;
        }

        buf++;
        size--;
    }

    return count;
}

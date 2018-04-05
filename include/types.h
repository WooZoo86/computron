// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef int8_t SIGNED_BYTE;
typedef int16_t SIGNED_WORD;
typedef int32_t SIGNED_DWORD;
typedef int64_t SIGNED_QWORD;

enum class SegmentRegisterIndex {
    ES = 0,
    CS,
    SS,
    DS,
    FS,
    GS,
    None = 0xFF,
};

enum ValueSize {
    ByteSize = 8,
    WordSize = 16,
    DWordSize = 32,
};

class PhysicalAddress {
public:
    PhysicalAddress() { }
    explicit PhysicalAddress(DWORD address) : m_address(address) { }

    DWORD get() const { return m_address; }
    void set(DWORD address) { m_address = address; }
    void mask(DWORD m) { m_address &= m; }

private:
    DWORD m_address { 0 };
};

class LinearAddress {
public:
    LinearAddress() { }
    explicit LinearAddress(DWORD address) : m_address(address) { }

    DWORD get() const { return m_address; }
    void set(DWORD address) { m_address = address; }
    void mask(DWORD m) { m_address &= m; }

private:
    DWORD m_address { 0 };
};

class LogicalAddress {
public:
    LogicalAddress() { }
    LogicalAddress(WORD selector, DWORD offset) : m_selector(selector), m_offset(offset) { }

    WORD selector() const { return m_selector; }
    DWORD offset() const { return m_offset; }
    void setSelector(WORD selector) { m_selector = selector; }
    void setOffset(DWORD offset) { m_offset = offset; }

private:
    WORD m_selector { 0 };
    DWORD m_offset { 0 };
};

template<typename T> struct BitSizeOfType { static const int bits = sizeof(T) * 8; };

template<typename T>
struct MasksForType
{
    static const T allBits = std::numeric_limits<typename std::make_unsigned<T>::type>::max();
};

template<typename T> struct TypeDoubler { };
template<> struct TypeDoubler<BYTE> { typedef WORD type; };
template<> struct TypeDoubler<WORD> { typedef DWORD type; };
template<> struct TypeDoubler<DWORD> { typedef QWORD type; };
template<> struct TypeDoubler<SIGNED_BYTE> { typedef SIGNED_WORD type; };
template<> struct TypeDoubler<SIGNED_WORD> { typedef SIGNED_DWORD type; };
template<> struct TypeDoubler<SIGNED_DWORD> { typedef SIGNED_QWORD type; };

template<typename T> struct TypeHalver { };
template<> struct TypeHalver<WORD> { typedef BYTE type; };
template<> struct TypeHalver<DWORD> { typedef WORD type; };
template<> struct TypeHalver<QWORD> { typedef DWORD type; };
template<> struct TypeHalver<SIGNED_WORD> { typedef SIGNED_BYTE type; };
template<> struct TypeHalver<SIGNED_DWORD> { typedef SIGNED_WORD type; };
template<> struct TypeHalver<SIGNED_QWORD> { typedef SIGNED_DWORD type; };

template<typename DT> constexpr DT weld(typename TypeHalver<DT>::type high, typename TypeHalver<DT>::type low)
{
    typedef typename std::make_unsigned<typename TypeHalver<DT>::type>::type UnsignedT;
    typedef typename std::make_unsigned<DT>::type UnsignedDT;
    return (((UnsignedDT)high) << BitSizeOfType<typename TypeHalver<DT>::type>::bits) | (UnsignedT)low;
}

template<typename T>
inline T signExtend(BYTE value)
{
    if (!(value & 0x80))
        return value;
    if (BitSizeOfType<T>::bits == 16)
        return value | 0xFF00;
    if (BitSizeOfType<T>::bits == 32)
        return value | 0xFFFFFF00;
    if (BitSizeOfType<T>::bits == 64)
        return value | 0xFFFFFFFFFFFFFF00;
}

template<typename T>
inline T signExtend(WORD value)
{
    if (!(value & 0x8000))
        return value;
    if (BitSizeOfType<T>::bits == 32)
        return value | 0xFFFF0000;
    if (BitSizeOfType<T>::bits == 64)
        return value | 0xFFFFFFFFFFFF0000;
}

inline WORD getMSW(DWORD d)
{
    return (d >> 16) & 0xFFFF;
}

inline WORD getLSW(DWORD d)
{
    return d & 0xFFFF;
}

inline BYTE getMSB(WORD w)
{
    return (w >> 8) & 0xFF;
}

inline BYTE getLSB(WORD w)
{
    return w & 0xFF;
}

inline WORD makeWORD(BYTE msb, BYTE lsb)
{
    return (msb << 8) | lsb;
}

inline DWORD makeDWORD(WORD msw, WORD lsw)
{
    return (msw << 16) | lsw;
}

inline QWORD makeQWORD(DWORD msw, DWORD lsw)
{
    return ((QWORD)msw << 32) | lsw;
}


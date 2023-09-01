/************************************************************************
 * Copyright (c) 2005-2006 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/
#include "set.h"

#include "m_exceptions.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>

namespace {
/**
 * @brief Returns a truncated word with the given number of bits
 * @param word Input word
 * @param nbits Number of bits to keep
 * @return Truncated word
 */
inline uint8_t truncate(uint8_t word, uint8_t nbits) noexcept
{
    return word & ((1 << nbits) - 1);
}

inline uint8_t tstbn(std::span<const uint8_t> buf, uint8_t x, uint8_t nbits) noexcept
{
    return buf[x >> nbits] & (1 << truncate(x, nbits));
}

inline void setbn(std::span<uint8_t> buf, uint8_t x, uint8_t nbits) noexcept
{
    buf[x >> nbits] |= (1 << truncate(x, nbits));
}

inline void clrbn(std::span<uint8_t> buf, uint8_t x, uint8_t nbits) noexcept
{
    buf[x >> nbits] &= ~(1 << truncate(x, nbits));
}

inline uint8_t tstb8(std::span<const uint8_t> buf, uint8_t x) noexcept
{
    return tstbn(buf, x, 3);
}

inline void setb8(std::span<uint8_t> buf, uint8_t x) noexcept
{
    setbn(buf, x, 3);
}

inline void clrb8(std::span<uint8_t> buf, uint8_t x) noexcept
{
    clrbn(buf, x, 3);
}

inline std::span<uint8_t> fitIntoByte(std::span<uint8_t> buf, uint8_t &pos) noexcept
{
    if (pos >= 8) {
        auto byte = buf.subspan(pos / 8);
        pos %= 8;
        return byte;
    } else {
        return buf;
    }
}

} // namespace

namespace Util {

// note: given a set of 1..N values
// this means one needs to malloc(N/8 + (n%8 == 0 ? 0 : 1) bytes of mem
// the bit status (reading from left to right) corresponds
// to the existence of the index number in the set

Set::Set()
{
    last = MAX_SET_COUNT;
    storage = new unsigned char[MAX_SET_COUNT / 8];
    memset((void *) storage, 0, (size_t) MAX_SET_COUNT / 8);
    ext_data = false;
}

Set::Set(int n)
{
    last = n;
    assert(n > 0);
    int k = n / 8 + int((n % 8) > 0);
    storage = new unsigned char[k];
    memset((void *) storage, 0, (size_t) k);
    ext_data = false;
}

Set::Set(const Set &other)
{
    last = other.get_last();
    unsigned char *os = other.storage; // WARNING: does not copy data
    assert(last > 0);
    int k = ((last % 8) > 0) ? last / 8 + 1 : last / 8;
    storage = new unsigned char[k];
    memcpy(storage, os, k);
    ext_data = false;
}

Set::Set(int k, unsigned char *data)
{
    storage = data;
    last = k;
    ext_data = true;
}

Set::~Set()
{
    if (!ext_data)
        delete[] storage;
}

int Set::get_last() const
{
    return last;
}

void Set::set_item(int k, bool val)
{
    if (k < last) {
        uint8_t bit = k;
        auto pos = fitIntoByte({ storage, (size_t) last }, bit);
        if (val)
            setb8(pos, bit);
        else
            clrb8(pos, bit);
    } else
        throw Util::OutOfRange(std::to_string(k) + " >= " + std::to_string(last));
}

void Set::set_items(int start, int end, bool val)
{
    for (int i = start; i < end; i++)
        set_item(i, val);
}


bool Set::get_item(int k) const
{
    if (k < last) {
        uint8_t bit = k;
        auto pos = fitIntoByte({ storage, (size_t) last }, bit);
        return tstb8(pos, bit);
    }
    throw Util::OutOfRange(std::to_string(k) + " >= " + std::to_string(last));
}

} // namespace Util

#include "bitwise.hpp"
#include <gtest/gtest.h>

// Generated with Copilot, seems legit
TEST(BitwiseTest, GetBit) {
    EXPECT_EQ(Util::getBit(0u, 1), false);
    EXPECT_EQ(Util::getBit(1u, 1), true);
    EXPECT_EQ(Util::getBit(1u, 2), false);
    EXPECT_EQ(Util::getBit(2u, 1), false);
    EXPECT_EQ(Util::getBit(2u, 2), true);
    EXPECT_EQ(Util::getBit(3u, 1), true);
    EXPECT_EQ(Util::getBit(3u, 2), true);
    EXPECT_EQ(Util::getBit(3u, 3), false);
    EXPECT_EQ(Util::getBit(4u, 1), false);
    EXPECT_EQ(Util::getBit(4u, 2), false);
    EXPECT_EQ(Util::getBit(4u, 3), true);
    EXPECT_EQ(Util::getBit(4u, 4), false);
    EXPECT_EQ(Util::getBit(5u, 1), true);
    EXPECT_EQ(Util::getBit(5u, 2), false);
    EXPECT_EQ(Util::getBit(5u, 3), true);
    EXPECT_EQ(Util::getBit(5u, 4), false);
    EXPECT_EQ(Util::getBit(6u, 1), false);
    EXPECT_EQ(Util::getBit(6u, 2), true);
    EXPECT_EQ(Util::getBit(6u, 3), true);
    EXPECT_EQ(Util::getBit(6u, 4), false);
    EXPECT_EQ(Util::getBit(7u, 1), true);
    EXPECT_EQ(Util::getBit(7u, 2), true);
    EXPECT_EQ(Util::getBit(7u, 3), true);
    EXPECT_EQ(Util::getBit(7u, 4), false);
    EXPECT_EQ(Util::getBit(8u, 1), false);
    EXPECT_EQ(Util::getBit(8u, 2), false);
    EXPECT_EQ(Util::getBit(8u, 3), false);
    EXPECT_EQ(Util::getBit(8u, 4), true);
    EXPECT_EQ(Util::getBit(9u, 1), true);
    EXPECT_EQ(Util::getBit(9u, 2), false);
    EXPECT_EQ(Util::getBit(9u, 3), false);
    EXPECT_EQ(Util::getBit(9u, 4), true);
    EXPECT_EQ(Util::getBit(10u, 1), false);
    EXPECT_EQ(Util::getBit(10u, 2), true);
}

// Generated with Copilot, seems legit
TEST(BitwiseTest, SetBit) {
    uint32_t value = 0;
    Util::setBit(&value, 1, true);
    EXPECT_EQ(value, 1);
    Util::setBit(&value, 1, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 2, true);
    EXPECT_EQ(value, 2);
    Util::setBit(&value, 2, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 3, true);
    EXPECT_EQ(value, 4);
    Util::setBit(&value, 3, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 4, true);
    EXPECT_EQ(value, 8);
    Util::setBit(&value, 4, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 5, true);
    EXPECT_EQ(value, 16);
    Util::setBit(&value, 5, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 6, true);
    EXPECT_EQ(value, 32);
    Util::setBit(&value, 6, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 7, true);
    EXPECT_EQ(value, 64);
    Util::setBit(&value, 7, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 8, true);
    EXPECT_EQ(value, 128);
    Util::setBit(&value, 8, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 9, true);
    EXPECT_EQ(value, 256);
    Util::setBit(&value, 9, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 10, true);
    EXPECT_EQ(value, 512);
    Util::setBit(&value, 10, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 11, true);
    EXPECT_EQ(value, 1024);
    Util::setBit(&value, 11, false);
    EXPECT_EQ(value, 0);
    Util::setBit(&value, 12, true);
    EXPECT_EQ(value, 2048);
    Util::setBit(&value, 12, false);
}

TEST(BitwiseTest, GetRangeBit) {
    EXPECT_EQ(Util::getRangeBit(0b0110u, 2, 3), 0b11);
    EXPECT_EQ(Util::getRangeBit(0b0110u, 1, 2), 0b10);
    EXPECT_EQ(Util::getRangeBit(0b0110u, 1, 3), 0b110);
    EXPECT_EQ(Util::getRangeBit(0b00010010001101000101011001111000u, 1, 8), 0b01111000);
    EXPECT_EQ(Util::getRangeBit(0b00010010001101000101011001111000u, 9, 16), 0b01010110);
    EXPECT_EQ(Util::getRangeBit(0b00010010001101000101011001111000u, 17, 24), 0b00110100);
    EXPECT_EQ(Util::getRangeBit(0b00010010001101000101011001111000u, 25, 32), 0b00010010);
}

TEST(BitwiseTest, SetRangeBit) {
    uint64_t value = 0;
    Util::setRangeBit(&value, 2, 3, true);
    EXPECT_EQ(value, 0b0110);
    Util::setRangeBit(&value, 1, 2, false);
    EXPECT_EQ(value, 0b0100);
    Util::setRangeBit(&value, 1, 3, true);
    EXPECT_EQ(value, 0b0111);
    Util::setRangeBit(&value, 1, 8, true);
    EXPECT_EQ(value, 0b11111111);
    Util::setRangeBit(&value, 9, 16, true);
    EXPECT_EQ(value, 0b1111111111111111);
    Util::setRangeBit(&value, 1, 8, false);
    EXPECT_EQ(value, 0b1111111100000000);
    Util::setRangeBit(&value, 17, 24, true);
    EXPECT_EQ(value, 0b111111111111111100000000);
}

TEST(BitwiseTest, CopyRangeBit) {
    uint32_t value = 0;
    Util::copyRangeBit(&value, 2, 3, 0b11u);
    EXPECT_EQ(value, 0b0110);
    Util::copyRangeBit(&value, 1, 2, 0b10u);
    EXPECT_EQ(value, 0b0110);
    Util::copyRangeBit(&value, 1, 3, 0b110u);
    EXPECT_EQ(value, 0b0110);
    Util::copyRangeBit(&value, 1, 8, 0b01111000u);
    EXPECT_EQ(value, 0b01111000);
    Util::copyRangeBit(&value, 9, 16, 0b01010110u);
    EXPECT_EQ(value, 0b0101011001111000u);
    Util::copyRangeBit(&value, 17, 24, 0b00110100u);
    EXPECT_EQ(value, 0b001101000101011001111000u);
    Util::copyRangeBit(&value, 25, 32, 0b00010010u);
    EXPECT_EQ(value, 0b00010010001101000101011001111000u);
}

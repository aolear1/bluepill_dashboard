#include <gtest/gtest.h>
#include "../Core/Inc/utils.h"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}

TEST(ChecksumTest, BasicAssertions) {
	uint8_t bruh[5];
	bruh[0] = 0xFF;
	bruh[1] = 0xAA;
	bruh[2] = 0xCC;
	bruh[3] = 0xBB;
	bruh[4] = 0xDD;

	uint8_t checksum_actual = bruh[0] ^ bruh[1] ^ bruh[2] ^ bruh[3] ^ bruh[4];
	uint8_t checksum;

	utilStatus status = calculateChecksum(bruh, sizeof(bruh), &checksum);

	EXPECT_EQ(checksum, checksum_actual);
	EXPECT_EQ(status, UTIL_SUCCESS);

	status = calculateChecksum(bruh, 0, &checksum);
	EXPECT_EQ(status, UTIL_INVALID_INPUT);

	status = calculateChecksum(0, 5, &checksum);
	EXPECT_EQ(status, UTIL_NULL_ERROR);

	status = calculateChecksum(bruh, 5, 0);
	EXPECT_EQ(status, UTIL_NULL_ERROR);
}

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

class ProcessPacketTest : public ::testing::Test {
protected:
    BMS_Params_t src{};
    BMS_Params_t dest{};
    uint8_t expected_checksum;

    void buildValidPacket() {
        src.header = configBMS_HEADER;
        uint8_t* start = reinterpret_cast<uint8_t*>(&src) + 1;

        uint8_t checksum = 0;
        for (size_t i = 0; i < sizeof(BMS_Params_t)-1; ++i) {
            start[i] = static_cast<uint8_t>(i + 3);
            checksum ^= static_cast<uint8_t>(i + 3);
        }

        calculateChecksum(start, sizeof(BMS_Params_t) - configBMS_CHECKSUM_PASSES, &src.checksum);
        expected_checksum = src.checksum;
    }
};

TEST_F(ProcessPacketTest, SucceedsWithValidChecksum) {
    buildValidPacket();

    EXPECT_EQ(processPacket(&src, &dest, sizeof(BMS_Params_t) - configBMS_CHECKSUM_PASSES),
              UTIL_SUCCESS);
    EXPECT_EQ(memcmp(&src, &dest, sizeof(BMS_Params_t)), 0);
}

TEST_F(ProcessPacketTest, FailsWithInvalidChecksum) {
    buildValidPacket();

    src.checksum = expected_checksum + 1;

    EXPECT_EQ(processPacket(&src, &dest, sizeof(BMS_Params_t) - configBMS_CHECKSUM_PASSES),
              UTIL_FAILED);
}

TEST_F(ProcessPacketTest, FailsWithInvalidHeader) {
    buildValidPacket();

    src.header = configBMS_HEADER - 1;

    EXPECT_EQ(processPacket(&src, &dest, sizeof(BMS_Params_t) - configBMS_CHECKSUM_PASSES),
              UTIL_FAILED);
}

#include <CppUTest/UtestMacros.h>
#include "CppUTest/TestHarness.h"

extern "C"
{
#include "protocol.h"
}

TEST_GROUP(TestGroupParseFrame)
{
	void setup()
	{
	}

	void teardown()
	{
	}
};

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsPartial)
{
    const char* buffer = "+Part";
    char* result = parse_frame(buffer);
    const char* expected = "";
	STRCMP_EQUAL(expected, (const char *)result);
}

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsWhole)
{
    const char* buffer = "+Full\r\n";
    char* result = parse_frame(buffer);
    const char* expected = "Full";
	STRCMP_EQUAL(expected, (const char *)result);
}

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsWholePartial)
{
    const char* buffer = "+Full\r\n+Part";
    char* result = parse_frame(buffer);
    const char* expected = "Full";
	STRCMP_EQUAL(expected, (const char *)result);
}

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsErrorPartial)
{
    const char* buffer = "-Err";
    char* result = parse_frame(buffer);
    const char* expected = "";
	STRCMP_EQUAL(expected, (const char *)result);
}

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsErrorWhole)
{
    const char* buffer = "-Err\r\n";
    char* result = parse_frame(buffer);
    const char* expected = "Err";
	STRCMP_EQUAL(expected, (const char *)result);
}

TEST(TestGroupParseFrame, TestParseFrameSimpleStringsErrorWholePartial)
{
    const char* buffer = "-Err\r\n+Part";
    char* result = parse_frame(buffer);
    const char* expected = "Err";
	STRCMP_EQUAL(expected, (const char *)result);
}

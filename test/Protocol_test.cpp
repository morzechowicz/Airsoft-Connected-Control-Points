#include <unity.h>  // PlatformIO's test framework
#include "../src/Protocol.h"

// Test setup (runs before each test)
void setUp(void) {
    // Nothing needed for protocol tests
}

// Test teardown (runs after each test)
void tearDown(void) {
    // Nothing needed
}

// ============================================
// PROTOCOL BUILDER TESTS
// ============================================

// void test_build_capture_message() {
//     String msg = Protocol::buildCapture(42, 1);
    
//     // Assert the message format is correct
//     TEST_ASSERT_EQUAL_STRING("CAP;42;1", msg.c_str());
// }

// void test_build_score_update() {
//     String msg = Protocol::buildScoreUpdate(100, 50);
    
//     TEST_ASSERT_EQUAL_STRING("UPD;100;50", msg.c_str());
// }

void test_build_game_start() {
    String msg = Protocol::buildGameStart();
    
    TEST_ASSERT_EQUAL_STRING("START", msg.c_str());
}

void test_build_game_over() {
    String msg = Protocol::buildGameOver(1);
    
    TEST_ASSERT_EQUAL_STRING("END;1", msg.c_str());
}

// ============================================
// PROTOCOL PARSER TESTS
// ============================================

void test_parse_capture_message() {
    String message = SYS + ";42;1";
    Message result;
    
    result = Protocol::parse(message.c_str(), message.length());
    
    TEST_ASSERT_EQUAL(SYS, result.type);
    TEST_ASSERT_EQUAL(2, result.paramCount);
    TEST_ASSERT_EQUAL(42, result.params[0].toInt());
    TEST_ASSERT_EQUAL(1, result.params[1].toInt());
}

void test_parse_score_update() {
    String message = GAME + ";100;50";
    Message result;
    
    result = Protocol::parse(message.c_str(), message.length());
    
    TEST_ASSERT_EQUAL(GAME, result.type);
    TEST_ASSERT_EQUAL(2, result.paramCount);
    TEST_ASSERT_EQUAL(100, result.params[0].toInt());
    TEST_ASSERT_EQUAL(50, result.params[1].toInt());
}

void test_parse_invalid_message() {
    String message = "";
    Message result;
    
    result = Protocol::parse(message.c_str(), message.length());
    
    TEST_ASSERT_EQUAL("", result.type);
}

void test_parse_malformed_message() {
    String message = ";;;";
    Message result;
    
    result = Protocol::parse(message.c_str(), message.length());
    
    // Should parse but have empty command
    TEST_ASSERT_EQUAL("", result.type);
    TEST_ASSERT_EQUAL_STRING(UNKNOWN, result.type);
}

// ============================================
// VALIDATION TESTS
// ============================================

// void test_validate_capture_message() {
//     Message msg;
//     Protocol::parse("CAP;42;1", msg);
    
//     TEST_ASSERT_TRUE(Protocol::validate(msg));
// }

// void test_validate_capture_missing_params() {
//     Protocol::ParsedMessage msg;
//     Protocol::parse("CAP;42", msg);  // Missing team ID
    
//     TEST_ASSERT_FALSE(Protocol::validate(msg));
// }

// void test_validate_unknown_command() {
//     Protocol::ParsedMessage msg;
//     Protocol::parse("UNKNOWN;1;2;3", msg);
    
//     // Unknown commands are considered valid (for extensibility)
//     TEST_ASSERT_TRUE(Protocol::validate(msg));
// }

// ============================================
// ROUNDTRIP TESTS (Build then Parse)
// ============================================

// void test_roundtrip_capture() {
//     // Build a message
//     String built = Protocol::buildCapture(99, 2);
    
//     // Parse it back
//     Protocol::ParsedMessage parsed;
//     Protocol::parse(built.c_str(), built.length(), parsed);
    
//     // Verify we get the same data back
//     TEST_ASSERT_EQUAL_STRING("CAP", parsed.command.c_str());
//     TEST_ASSERT_EQUAL(99, parsed.getIntParam(0));
//     TEST_ASSERT_EQUAL(2, parsed.getIntParam(1));
// }

// void test_roundtrip_score() {
//     String built = Protocol::buildScoreUpdate(123, 456);
    
//     Protocol::ParsedMessage parsed;
//     Protocol::parse(built, parsed);
    
//     TEST_ASSERT_EQUAL_STRING("UPD", parsed.command.c_str());
//     TEST_ASSERT_EQUAL(123, parsed.getIntParam(0));
//     TEST_ASSERT_EQUAL(456, parsed.getIntParam(1));
// }

// ============================================
// MAIN TEST RUNNER
// ============================================

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();  // Start testing
    
    // Protocol builder tests
    // RUN_TEST(test_build_capture_message);
    // RUN_TEST(test_build_score_update);
    RUN_TEST(test_build_game_start);
    RUN_TEST(test_build_game_over);
    
    // Protocol parser tests
    RUN_TEST(test_parse_capture_message);
    RUN_TEST(test_parse_score_update);
    RUN_TEST(test_parse_invalid_message);
    RUN_TEST(test_parse_malformed_message);
    
    // // Validation tests
    // RUN_TEST(test_validate_capture_message);
    // RUN_TEST(test_validate_capture_missing_params);
    // RUN_TEST(test_validate_unknown_command);
    
    // // Roundtrip tests
    // RUN_TEST(test_roundtrip_capture);
    // RUN_TEST(test_roundtrip_score);
    
    UNITY_END();  // Finish testing
}

void loop() {
    // Nothing here - tests run once in setup()
}
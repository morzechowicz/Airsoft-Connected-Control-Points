#include <unity.h>
#include <StringSplitter.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_splitting()
{
    StringSplitter splitter;
    String msg = "0/1/2/3/4/5/6";
    splitter.split(msg);

    TEST_ASSERT_EQUAL_STRING("0/", splitter.getItem(0).c_str());
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    RUN_TEST(test_splitting);
    UNITY_END();
}

void setup() {
    delay(2000);
    RUN_UNITY_TESTS();
}

void loop() {
    // Nothing to do here
}
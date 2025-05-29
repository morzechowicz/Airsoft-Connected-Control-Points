#include <unity.h>
#include <ControlPoint.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_control_point_initial_state() {
    ControlPoint cp(2);
    TEST_ASSERT_EQUAL(TeamId::None, cp.getControllingTeam());
    TEST_ASSERT_EQUAL(0, cp.getNodeCount());
}

void test_add_team() {
    ControlPoint cp(2);
    cp.addTeam(TeamId::Blufor);
    TEST_ASSERT_EQUAL(1, cp.getTeamPoints(TeamId::Blufor));
}

void test_set_controlling_team() {
    ControlPoint cp(2);
    cp.setNodeId(1);
    cp.addNode(1);
    cp.setControllingTeam(TeamId::Blufor, 1);
    TEST_ASSERT_EQUAL(TeamId::Blufor, cp.getControllingTeam());
}

void test_change_controlling_team_wrong_button()
{
    ControlPoint cp(2);
    cp.addTeam(TeamId::Blufor);
    cp.addTeam(TeamId::YellowFor);
    cp.setNodeId(1);
    cp.addNode(1);
    cp.setControllingTeam(TeamId::Blufor, 1);
    cp.setControllingTeam(TeamId::Blufor, 1);
    TEST_ASSERT_NOT_EQUAL(TeamId::YellowFor, cp.getControllingTeam())
    TEST_ASSERT_EQUAL(TeamId::Blufor, cp.getControllingTeam());
}

void test_change_with_two_nodes()
{
    ControlPoint cp(2);
    cp.addTeam(TeamId::Blufor);
    cp.addTeam(TeamId::YellowFor);
    cp.setNodeId(1);
    cp.addNode(1);
    cp.addNode(2);
    
    cp.setControllingTeam(TeamId::Blufor,1);
    cp.setControllingTeam(TeamId::Blufor,2);
    TEST_ASSERT_EQUAL(TeamId::Blufor, cp.getControllingTeam());
    TEST_ASSERT_EQUAL(TeamId::Blufor, cp.getControllingTeam());
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    RUN_TEST(test_control_point_initial_state);
    RUN_TEST(test_add_team);
    RUN_TEST(test_set_controlling_team);
    RUN_TEST(test_change_controlling_team_wrong_button);
    UNITY_END();
}

void setup() {
    delay(2000);
    RUN_UNITY_TESTS();
}

void loop() {
    // Nothing to do here
}
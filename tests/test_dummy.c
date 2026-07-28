/**
 * @file test_dummy.c
 * @brief Simple assertion test suite to verify testing pipeline.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "agent.h"

static void test_agent_constants(void) {
    assert(strcmp(AGENT_VERSION, "0.1.0") == 0);
    assert(strcmp(AGENT_NAME, "agent-egg-kch") == 0);
    printf("[PASS] test_agent_constants\n");
}

int main(void) {
    printf("Running unit tests...\n");
    test_agent_constants();
    printf("All unit tests passed successfully!\n");
    return 0;
}

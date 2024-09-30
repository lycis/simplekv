#include <stdio.h>
#include "cmunit.h"

char* test_example_passed() {
    cmunit_assert("Test failed", 1 == 1);
    return NULL;
}

char* test_example_failed() {
    cmunit_assert("Test failed", 1 == 2);
    return NULL;
}

int main(void) {
    cmunit_init();

    cmunit_run_test(test_example_passed);
    cmunit_run_test(test_example_failed);

    cmunit_summary();

    return _cmunit_test_errors;
}
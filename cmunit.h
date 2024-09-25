// C Macro Unit Test Framework
// based upon the idea of MinUnit (http://www.jera.com/techinfo/jtns/jtn002.html) and extended with some convenience macros.
# ifndef CMUNIT_H
# define CMUNIT_H

 #define cmunit_assert(message, test) \
    do { \
        if (!(test)) return message;\
    } while (0)

 #define cmunit_run_test(test) \
    do { \
        char *message = test(); \
        char fname[256]; \
        sprintf(fname, "%-70s", #test); \
        _cmunit_tests_run++; \
        if (message != NULL) {\
            _cmunit_test_errors++; \
            printf("%s ❌ failed: %s\n", fname, message); \
        } \
        else { \
            printf("%s ✅ passed\n", fname); \
        } \
    } while (0)

#define cmunit_init() \
    do { \
        _cmunit_tests_run = 0; \
        _cmunit_test_errors = 0; \
        printf("cmunit - Running Tests\n"); \
    } while (0)

#define cmunit_summary() \
    do { \
        printf("Passed: %d Failed: %d\n", _cmunit_tests_run - _cmunit_test_errors, _cmunit_test_errors); \
    } while (0)

int _cmunit_tests_run;
int _cmunit_test_errors;
#endif
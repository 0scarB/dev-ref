#define _POSIX_C_SOURCE 1

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//
// Library
//

char* cursedpad(char* str, size_t len, char ch) {
    char* result = malloc(len + 1);
    int prefix_len = len - strlen(str);
    if (prefix_len < 0)
        prefix_len = 0;
    memset(result, ch, prefix_len);
    memcpy(result + prefix_len, str, len - prefix_len);
    result[len] = '\0';
    return result;
}

#ifndef TEST

//
// Application
//

int main(void) {
    printf("%s\n", cursedpad(" leftpad", 30, 'A'));

    return EXIT_SUCCESS;
}

#endif
#ifdef TEST

//
// Test Framework Core
//

typedef enum test_node_type {
    TEST_NODE_GROUP_SUCCESS,
    TEST_NODE_GROUP_FAILED,
    TEST_NODE_ASSERT_FAILED,
} TestNodeType;

typedef struct test_node {
    TestNodeType type;
    int          depth;
    char         msg[1024];
} TestNode;

TestNode  test_nodes[1024] = {0};
size_t    test_nodes_count =  0;
TestNode* test_stack[16]   = {0};
size_t    test_stack_depth =  0;

TestNode* test_node_add(TestNodeType type) {
    TestNode* test_node = test_nodes + test_nodes_count;
    test_node->type  = type;
    test_node->depth = test_stack_depth;

    test_stack[test_stack_depth] = test_node;

    ++test_nodes_count;
    ++test_stack_depth;

    return test_node;
}

void test_begin(char* group_description) {
    TestNode* test_node = test_node_add(TEST_NODE_GROUP_SUCCESS);
    strcpy(test_node->msg, group_description);
}

void test_end(void) {
    --test_stack_depth;
    if (test_stack[test_stack_depth]->type == TEST_NODE_GROUP_SUCCESS)
        return;

    for (int i = test_stack_depth - 1; i > -1; --i)
        test_stack[i]->type = TEST_NODE_GROUP_FAILED;
}

void test_assert(bool condition, char* fmt, ...) {
    if (condition)
        return;

    TestNode* test_node = test_node_add(TEST_NODE_ASSERT_FAILED);

    va_list  sprintf_args;
    va_start(sprintf_args, fmt);
    vsprintf(test_node->msg, fmt, sprintf_args);
    va_end  (sprintf_args);

    test_end();
}

int test_report_returning_exit_status(void) {
    const char* INDENT_PATTERN = "│   │   │   │   │   │   │   │   │   │   │   │   ";

    bool exit_status = EXIT_SUCCESS;

    for (size_t i = 0; i < test_nodes_count; ++i) {
        TestNode node = test_nodes[i];
        switch (node.type) {
            case TEST_NODE_GROUP_SUCCESS:
                printf(
                    "%.*s✔ %s\n"  ,  node.depth*6   , INDENT_PATTERN, node.msg);
                break;
            case TEST_NODE_GROUP_FAILED:
                fprintf(stderr,
                    "%.*s✘ %s\n"  ,  node.depth*6   , INDENT_PATTERN, node.msg);
                exit_status = EXIT_FAILURE;
                break;
            case TEST_NODE_ASSERT_FAILED:
                fprintf(stderr,
                    "%.*s├── %s\n", (node.depth-1)*6, INDENT_PATTERN, node.msg);
                exit_status = EXIT_FAILURE;
                break;
        }
    }

    return exit_status;
}

//
// Test Framework Extensions
//

void test_assert_strs_eq(char* left, char* right) {
    test_assert(strcmp(left, right) == 0, "'%s' == '%s'", left, right);
}

// Add more extensions for your use case here!


//
// Test Framework
//

int main(void) {
    test_begin("cursedpad(str, len, ch)"); {
        test_begin("where len >  strlen(str)"); {
            char* padded_str = cursedpad(" leftpad", 12, 'A');
            test_assert_strs_eq(padded_str, "AAAA leftpad");
            free(padded_str);
        }; test_end();

        test_begin("where len == strlen(str)"); {
            char* padded_str = cursedpad(" leftpad", 8, 'A');
            test_assert_strs_eq(padded_str, " leftpad");
            free(padded_str);
        }; test_end();

        test_begin("where len <  strlen(str)"); {
            char* padded_str = cursedpad(" leftpad", 5, 'A');
            test_assert_strs_eq(padded_str, " leftpad");
            free(padded_str);
        }; test_end();
    }; test_end();

    return test_report_returning_exit_status();
}

#endif


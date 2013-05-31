#include <check.h>
#include <stdint.h>
#include "atcommander.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

AtCommanderConfig config;

void debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void baud_rate_initializer(int baud) {
}

void mock_write(uint8_t byte) {
}

static char* read_message;
static int read_message_length;
static int read_index;

int mock_read() {
    if(read_message != NULL && read_index < read_message_length) {
        return read_message[read_index++];
    }
    return -1;
}

void setup() {
    config.platform = AT_PLATFORM_RN42;
    config.connected = false;
    config.baud = 9600;
    config.device_baud = 9600;
    config.baud_rate_initializer = baud_rate_initializer;
    config.write_function = mock_write;
    config.read_function = mock_read;
    config.delay_function = NULL;
    config.log_function = debug;

    read_message = NULL;
    read_message_length = 0;
    read_index = 0;
}


START_TEST (test_enter_command_mode_success)
{
    char* response = "CMD\r\n";
    read_message = response;
    read_message_length = 5;

    ck_assert(!config.connected);
    ck_assert(at_commander_enter_command_mode(&config));
    ck_assert(config.connected);
}
END_TEST

START_TEST (test_enter_command_mode_already_active)
{
    read_message = NULL;
    read_message_length = 0;

    config.connected = true;
    ck_assert(at_commander_enter_command_mode(&config));
    ck_assert(config.connected);
}
END_TEST

START_TEST (test_enter_command_mode_at_baud)
{
    char* response = "BADAABADAACMD\r\n";
    read_message = response;
    read_message_length = 15;

    ck_assert(!config.connected);
    ck_assert(at_commander_enter_command_mode(&config));
    ck_assert(config.connected);
}
END_TEST

START_TEST (test_enter_command_mode_fail_bad_response)
{
    char* response = "BAD\r\n";
    read_message = response;
    read_message_length = 5;

    ck_assert(!config.connected);
    ck_assert(!at_commander_enter_command_mode(&config));
    ck_assert(!config.connected);
}
END_TEST

START_TEST (test_enter_command_mode_fail_no_response)
{
    read_message = NULL;
    read_message_length = 0;

    ck_assert(!config.connected);
    ck_assert(!at_commander_enter_command_mode(&config));
    ck_assert(!config.connected);
}
END_TEST

START_TEST (test_exit_command_mode_success)
{
    char* response = "END\r\n";
    read_message = response;
    read_message_length = 5;

    config.connected = true;
    at_commander_exit_command_mode(&config);
    ck_assert(!config.connected);
}
END_TEST

START_TEST (test_exit_command_mode_fail_bad_response)
{
    char* response = "CMD";
    read_message = response;
    read_message_length = 3;

    config.connected = true;
    at_commander_exit_command_mode(&config);
    ck_assert(config.connected);
}
END_TEST

START_TEST (test_exit_command_mode_fail_no_response)
{
    read_message = NULL;
    read_message_length = 0;

    config.connected = true;
    at_commander_exit_command_mode(&config);
    ck_assert(config.connected);
}
END_TEST

START_TEST (test_exit_command_mode_already_done)
{
    read_message = NULL;
    read_message_length = 0;

    ck_assert(!config.connected);
    at_commander_exit_command_mode(&config);
    ck_assert(!config.connected);
}
END_TEST

START_TEST (test_set_baud_success)
{
    char* response = "CMD\r\nAOK\r\n";
    read_message = response;
    read_message_length = 10;

    ck_assert(!config.connected);
    ck_assert(at_commander_set_baud(&config, 115200));
    ck_assert(config.connected);
    ck_assert_int_eq(config.device_baud, 115200);
}
END_TEST

START_TEST (test_set_baud_bad_response)
{
    char* response = "CMD\r\n?";
    read_message = response;
    read_message_length = 6;

    ck_assert(!config.connected);
    ck_assert_int_ne(config.device_baud, 115200);
    ck_assert(!at_commander_set_baud(&config, 115200));
    ck_assert(config.connected);
    ck_assert_int_ne(config.device_baud, 115200);
}
END_TEST

START_TEST (test_set_baud_no_response)
{
    char* response = "CMD\r\n";
    read_message = response;
    read_message_length = 5;

    ck_assert(!config.connected);
    ck_assert_int_ne(config.device_baud, 115200);
    ck_assert(!at_commander_set_baud(&config, 115200));
    ck_assert(config.connected);
    ck_assert_int_ne(config.device_baud, 115200);
}
END_TEST

Suite* suite(void) {
    Suite* s = suite_create("atcommander");
    TCase *tc_enter_command_mode = tcase_create("enter_command_mode");
    tcase_add_checked_fixture(tc_enter_command_mode, setup, NULL);
    tcase_add_test(tc_enter_command_mode, test_enter_command_mode_success);
    tcase_add_test(tc_enter_command_mode, test_enter_command_mode_already_active);
    tcase_add_test(tc_enter_command_mode, test_enter_command_mode_fail_bad_response);
    tcase_add_test(tc_enter_command_mode, test_enter_command_mode_fail_no_response);
    tcase_add_test(tc_enter_command_mode, test_enter_command_mode_at_baud);
    suite_add_tcase(s, tc_enter_command_mode);

    TCase *tc_exit_command_mode = tcase_create("exit_command_mode");
    tcase_add_checked_fixture(tc_exit_command_mode, setup, NULL);
    tcase_add_test(tc_exit_command_mode, test_exit_command_mode_success);
    tcase_add_test(tc_exit_command_mode, test_exit_command_mode_fail_bad_response);
    tcase_add_test(tc_exit_command_mode, test_exit_command_mode_fail_no_response);
    tcase_add_test(tc_exit_command_mode, test_exit_command_mode_already_done);
    suite_add_tcase(s, tc_exit_command_mode);

    TCase *tc_set_baud = tcase_create("set_baud");
    tcase_add_checked_fixture(tc_set_baud, setup, NULL);
    tcase_add_test(tc_set_baud, test_set_baud_success);
    tcase_add_test(tc_set_baud, test_set_baud_bad_response);
    tcase_add_test(tc_set_baud, test_set_baud_no_response);
    suite_add_tcase(s, tc_set_baud);
    return s;
}

int main(void) {
    int numberFailed;
    Suite* s = suite();
    SRunner *sr = srunner_create(s);
    // Don't fork so we can actually use gdb
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    numberFailed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (numberFailed == 0) ? 0 : 1;
}

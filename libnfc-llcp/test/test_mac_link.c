#include <err.h>
#include <cutter.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <nfc/nfc.h>

#include "llc_link.h"
#include "mac.h"

#define INITIATOR 0
#define TARGET    1

pthread_t threads[2];
nfc_device_desc_t device_descriptions[2];
nfc_device_t *devices[2];
intptr_t result[2];

struct test_thread_data {
    CutTestContext *context;
    nfc_device_t *device;
};

void
abort_test_by_keypress (int sig)
{
    (void) sig;
    printf ("\033[0;1;31mSIGINT\033[0m");

    nfc_abort_command (devices[INITIATOR]);
    nfc_abort_command (devices[TARGET]);
}

void
cut_setup (void)
{
    size_t n;

    llcp_init ();

    //cut_pend ("MAC link over DEP does not work");

    nfc_list_devices (device_descriptions, 2, &n);
    if (n < 2) {
	cut_omit ("At least two NFC devices must be plugged-in to run this test");
    }

    devices[TARGET] = nfc_connect (&device_descriptions[TARGET]);
    devices[INITIATOR] = nfc_connect (&device_descriptions[INITIATOR]);
}

void
cut_teardown (void)
{
    nfc_disconnect (devices[TARGET]);
    nfc_disconnect (devices[INITIATOR]);

    llcp_fini ();
}

void *
target_thread (void *arg)
{
    intptr_t res = 0;
    struct test_thread_data *thread_data = (struct test_thread_data *) arg;
    /*
    uint8_t id[] = {
	0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0x00, 0x00
    };*/

    cut_set_current_test_context (thread_data->context);

    printf ("Activating target...\n");
    struct llc_link *llc_link = llc_link_new ();
    cut_assert_not_null (llc_link, cut_message ("llc_link_new() failed"));

    struct mac_link *link = mac_link_new (thread_data->device, llc_link);
    cut_assert_not_null (link, cut_message ("mac_link_new() failed"));

    res = mac_link_activate_as_target (link);
    cut_assert_equal_int (1, res, cut_message ("mac_link_activate_as_target() failed"));

    sleep (3);

    printf ("===== DEACTIVATE =====\n");
    mac_link_deactivate (link, MAC_DEACTIVATE_ON_REQUEST);
    mac_link_free (link);
    llc_link_free (llc_link);
    return (void *) res;
}

void *
initiator_thread (void *arg)
{
    intptr_t res = 0;
    struct test_thread_data *thread_data = (struct test_thread_data *) arg;

    cut_set_current_test_context (thread_data->context);

    struct llc_link *llc_link = llc_link_new ();
    cut_assert_not_null (llc_link, cut_message ("llc_link_new() failed"));

    struct mac_link *link = mac_link_new (thread_data->device, llc_link);
    cut_assert_not_null (link, cut_message ("mac_link_new() failed"));

    printf ("Activating initiator...\n");
    res = mac_link_activate_as_initiator (link);
    cut_assert_equal_int (1, res, cut_message ("mac_link_activate_as_initiator() failed"));

    printf ("===== DEACTIVATE =====\n");
    mac_link_deactivate (link, MAC_DEACTIVATE_ON_REQUEST);
    mac_link_free (link);
    llc_link_free (llc_link);
    return (void *) res;
}

void
test_mac_link (void)
{
    int res;

    struct test_thread_data thread_data[2];

    thread_data[INITIATOR].context = thread_data[TARGET].context = cut_get_current_test_context ();
    thread_data[INITIATOR].device = devices[INITIATOR];
    thread_data[TARGET].device = devices[TARGET];

    if ((res = pthread_create (&(threads[TARGET]), NULL, target_thread, &thread_data[TARGET])))
	cut_fail ("pthread_create() returned %d", res);
    if ((res = pthread_create (&(threads[INITIATOR]), NULL, initiator_thread, &thread_data[INITIATOR])))
	cut_fail ("pthread_create() returned %d", res);

    signal (SIGINT, abort_test_by_keypress);

    if ((res = pthread_join (threads[INITIATOR], (void *) &result[INITIATOR])))
	cut_fail ("pthread_join() returned %d", res);
    if ((res = pthread_join (threads[TARGET], (void *) &result[TARGET])))
	cut_fail ("pthread_join() returned %d", res);

    cut_assert_equal_int (0, result[INITIATOR], cut_message ("Unexpected initiator return code"));
    cut_assert_equal_int (0, result[TARGET], cut_message ("Unexpected target return code"));
}

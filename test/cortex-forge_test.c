#include "libcortex-forge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

static int passed = 0, failed = 0;
#define DEVICE "/dev/cortex-forge0"
#define T(n) do { printf("  TEST: %-45s ", n); fflush(stdout); } while (0)
#define P() do { printf("PASS\n"); passed++; } while (0)
#define F(m) do { printf("FAIL: %s\n", m); failed++; } while (0)
#define S(r) do { printf("SKIP (%s)\n", r); } while (0)

static void t_open_close(void) {
    T("open and close device");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    cortex_forge_close(h); P();
}

static void t_submit_dla0(void) {
    T("submit task to DLA0");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
    d.accel_type = 0; d.priority = 128; d.input_size = 4096; d.output_size = 4096;
    int r = cortex_forge_submit_task(h, &d);
    if (r) { cortex_forge_close(h); F("submit failed"); return; }
    if (d.task_id == 0) { cortex_forge_close(h); F("no task_id"); return; }
    cortex_forge_close(h); P();
}

static void t_submit_dla1(void) {
    T("submit task to DLA1");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
    d.accel_type = 1; d.priority = 64; d.input_size = 8192; d.output_size = 8192;
    int r = cortex_forge_submit_task(h, &d);
    if (r) { cortex_forge_close(h); F("submit failed"); return; }
    cortex_forge_close(h); P();
}

static void t_submit_pva(void) {
    T("submit task to PVA");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
    d.accel_type = 2; d.priority = 255; d.input_size = 2048; d.output_size = 2048;
    int r = cortex_forge_submit_task(h, &d);
    if (r) { cortex_forge_close(h); F("submit failed"); return; }
    cortex_forge_close(h); P();
}

static void t_submit_query(void) {
    T("submit and query task");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
    d.accel_type = 0; d.input_size = 1024; d.output_size = 1024;
    int r = cortex_forge_submit_task(h, &d);
    if (r) { cortex_forge_close(h); F("submit"); return; }
    struct cortex_forge_task_status s; memset(&s, 0, sizeof(s)); s.task_id = d.task_id;
    r = cortex_forge_query_task(h, &s);
    if (r) { cortex_forge_close(h); F("query"); return; }
    if (s.task_id != d.task_id) { cortex_forge_close(h); F("id mismatch"); return; }
    cortex_forge_close(h); P();
}

static void t_submit_cancel(void) {
    T("submit and cancel task");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
    d.accel_type = 0; d.timeout_ms = 5000;
    int r = cortex_forge_submit_task(h, &d);
    if (r) { cortex_forge_close(h); F("submit"); return; }
    r = cortex_forge_cancel_task(h, d.task_id);
    if (r) { cortex_forge_close(h); F("cancel"); return; }
    cortex_forge_close(h); P();
}

static void t_get_info_all(void) {
    T("get info for all accelerators");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    for (int i = 0; i < 3; i++) {
        struct cortex_forge_accel_info info; memset(&info, 0, sizeof(info)); info.accel_type = i;
        int r = cortex_forge_get_accel_info(h, &info);
        if (r) { cortex_forge_close(h); F("get_info failed"); return; }
    }
    cortex_forge_close(h); P();
}

static void t_get_version(void) {
    T("get driver version");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    uint32_t v; int r = cortex_forge_get_version(h, &v);
    if (r || v == 0) { cortex_forge_close(h); F("get_version failed"); return; }
    cortex_forge_close(h); P();
}

static void t_set_power(void) {
    T("set power mode");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    int r = cortex_forge_set_power(h, 1);
    if (r) { cortex_forge_close(h); F("set_power failed"); return; }
    cortex_forge_close(h); P();
}

static void t_invalid_type(void) {
    T("reject invalid accelerator type");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d)); d.accel_type = 99;
    int r = cortex_forge_submit_task(h, &d);
    if (r != -EINVAL) { cortex_forge_close(h); F("expected -EINVAL"); return; }
    cortex_forge_close(h); P();
}

static void t_query_nonexist(void) {
    T("query non-existent task");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    struct cortex_forge_task_status s; memset(&s, 0, sizeof(s)); s.task_id = 999999;
    int r = cortex_forge_query_task(h, &s);
    if (r != -ENOENT) { cortex_forge_close(h); F("expected -ENOENT"); return; }
    cortex_forge_close(h); P();
}

static void t_cancel_nonexist(void) {
    T("cancel non-existent task");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    int r = cortex_forge_cancel_task(h, 999999);
    if (r != -ENOENT) { cortex_forge_close(h); F("expected -ENOENT"); return; }
    cortex_forge_close(h); P();
}

static void t_null_handle(void) {
    T("NULL handle returns -EINVAL");
    if (cortex_forge_submit_task(NULL, NULL) != -EINVAL) { F("submit"); return; }
    if (cortex_forge_query_task(NULL, NULL) != -EINVAL) { F("query"); return; }
    if (cortex_forge_cancel_task(NULL, 0) != -EINVAL) { F("cancel"); return; }
    if (cortex_forge_get_accel_info(NULL, NULL) != -EINVAL) { F("get_info"); return; }
    if (cortex_forge_get_version(NULL, NULL) != -EINVAL) { F("get_version"); return; }
    P();
}

static void t_multiple(void) {
    T("submit 10 tasks and query all");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    uint32_t ids[10];
    for (int i = 0; i < 10; i++) {
        struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
        d.accel_type = i % 3; d.priority = i * 10; d.input_size = 1024; d.output_size = 1024;
        int r = cortex_forge_submit_task(h, &d);
        if (r) { cortex_forge_close(h); F("submit"); return; }
        ids[i] = d.task_id;
    }
    for (int i = 0; i < 10; i++) {
        struct cortex_forge_task_status s; memset(&s, 0, sizeof(s)); s.task_id = ids[i];
        int r = cortex_forge_query_task(h, &s);
        if (r) { cortex_forge_close(h); F("query"); return; }
    }
    cortex_forge_close(h); P();
}

static void t_strerror(void) {
    T("strerror messages");
    if (strcmp(cortex_forge_strerror(0), "Success") != 0) { F("Success"); return; }
    if (strlen(cortex_forge_strerror(-EINVAL)) == 0) { F("empty"); return; }
    if (strlen(cortex_forge_strerror(-ENOMEM)) == 0) { F("empty"); return; }
    P();
}

struct td { struct cortex_forge_handle *h; int ops; int errs; };
static void *worker(void *a) {
    struct td *t = a;
    for (int i = 0; i < t->ops; i++) {
        struct cortex_forge_task_desc d; memset(&d, 0, sizeof(d));
        d.accel_type = rand() % 3; d.priority = rand() % 256; d.input_size = 1024; d.output_size = 1024;
        int r = cortex_forge_submit_task(t->h, &d);
        if (r == 0) { struct cortex_forge_task_status s; memset(&s, 0, sizeof(s)); s.task_id = d.task_id; cortex_forge_query_task(t->h, &s); }
        else t->errs++;
    }
    return NULL;
}

static void t_concurrent(void) {
    T("concurrent access 4 threads");
    struct cortex_forge_handle *h = cortex_forge_open(DEVICE);
    if (!h) { S("driver not loaded"); return; }
    pthread_t th[4]; struct td td[4]; int total = 0;
    for (int i = 0; i < 4; i++) { td[i].h = h; td[i].ops = 25; td[i].errs = 0; pthread_create(&th[i], NULL, worker, &td[i]); }
    for (int i = 0; i < 4; i++) { pthread_join(th[i], NULL); total += td[i].errs; }
    cortex_forge_close(h);
    if (total > 0) { F("concurrent errors"); return; }
    P();
}

int main(void) {
    printf("Cortex Forge Driver Test Suite\n==============================\n\n");
    t_open_close(); t_submit_dla0(); t_submit_dla1(); t_submit_pva();
    t_submit_query(); t_submit_cancel(); t_get_info_all(); t_get_version();
    t_set_power(); t_invalid_type(); t_query_nonexist(); t_cancel_nonexist();
    t_null_handle(); t_multiple(); t_strerror(); t_concurrent();
    printf("\nResults: %d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}

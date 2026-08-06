#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <cerrno>

TEST_CASE("Null input handling", "[edge]") {
    // Verify that the userspace library rejects NULL handles
    struct cortex_forge_task_desc desc;
    std::memset(&desc, 0, sizeof(desc));
    int ret = cortex_forge_submit_task(NULL, &desc);
    REQUIRE(ret == -EINVAL);

    ret = cortex_forge_query_task(NULL, NULL);
    REQUIRE(ret == -EINVAL);

    ret = cortex_forge_cancel_task(NULL, 0);
    REQUIRE(ret == -EINVAL);
}

TEST_CASE("Empty input handling", "[edge]") {
    // Verify that zero-size submissions are handled
    struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
    if (!h) { SKIP("driver not loaded"); return; }

    struct cortex_forge_task_desc desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.accel_type = 0;
    desc.input_size = 0;
    desc.output_size = 0;
    int ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == 0);
    REQUIRE(desc.task_id > 0);

    cortex_forge_close(h);
}

TEST_CASE("Boundary values", "[edge]") {
    // Verify boundary conditions for accelerator types and sizes
    struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
    if (!h) { SKIP("driver not loaded"); return; }

    struct cortex_forge_task_desc desc;
    std::memset(&desc, 0, sizeof(desc));

    // Max valid accelerator type
    desc.accel_type = 2; // PVA
    desc.input_size = 1024;
    desc.output_size = 1024;
    int ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == 0);

    // Invalid accelerator type
    desc.accel_type = 99;
    ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == -EINVAL);

    // Max priority
    desc.accel_type = 0;
    desc.priority = 255;
    ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == 0);

    cortex_forge_close(h);
}

TEST_CASE("Concurrent access", "[edge]") {
    // Verify thread safety with concurrent task submissions
    struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
    if (!h) { SKIP("driver not loaded"); return; }

    constexpr int NUM_THREADS = 4;
    constexpr int OPS_PER_THREAD = 25;
    std::atomic<int> errors{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([h, &errors]() {
            for (int i = 0; i < OPS_PER_THREAD; ++i) {
                struct cortex_forge_task_desc desc;
                std::memset(&desc, 0, sizeof(desc));
                desc.accel_type = rand() % 3;
                desc.priority = rand() % 256;
                desc.input_size = 1024;
                desc.output_size = 1024;
                int ret = cortex_forge_submit_task(h, &desc);
                if (ret != 0) errors++;
            }
        });
    }

    for (auto& t : threads) t.join();
    REQUIRE(errors == 0);

    cortex_forge_close(h);
}

TEST_CASE("Resource cleanup on error", "[edge]") {
    // Verify that resources are cleaned up on submission failure
    struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
    if (!h) { SKIP("driver not loaded"); return; }

    struct cortex_forge_task_desc desc;
    std::memset(&desc, 0, sizeof(desc));

    // Submit with invalid flags should fail without leaking
    desc.accel_type = 0;
    desc.flags = 0xDEAD;
    int ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == -EINVAL);

    // Subsequent valid submission should still work
    desc.flags = 0;
    ret = cortex_forge_submit_task(h, &desc);
    REQUIRE(ret == 0);

    cortex_forge_close(h);
}

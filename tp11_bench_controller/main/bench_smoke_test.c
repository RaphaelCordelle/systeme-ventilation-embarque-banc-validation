#include "bench_smoke_test.h"

#include <stdint.h>

#include "esp_log.h"

#include "bench_sensor.h"
#include "bench_protocol.h"

static const char *TAG = "BENCH_SMOKE";

bool bench_smoke_test_run(void)
{
    bool sensor_ok =
        false;

    bool dut_ok =
        false;

    bench_measurement_t reference =
        bench_sensor_read();

    if (reference.valid)
    {
        sensor_ok =
            true;

        ESP_LOGI(
            TAG,
            "Reference sensor OK"
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "Reference sensor FAIL"
        );
    }

    uint8_t state = 0;

    if (bench_get_dut_state(&state))
    {
        dut_ok =
            true;

        ESP_LOGI(
            TAG,
            "DUT communication OK state=%u",
            (unsigned int)state
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "DUT communication FAIL"
        );
    }

    if (sensor_ok && dut_ok)
    {
        ESP_LOGI(
            TAG,
            "SMOKE TEST PASS"
        );

        return true;
    }

    ESP_LOGE(
        TAG,
        "SMOKE TEST FAIL"
    );

    return false;
}

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bench_uart.h"
#include "bench_version.h"
#include "bench_sensor.h"
#include "bench_protocol.h"
#include "bench_compare.h"
#include "bench_smoke_test.h"

static const char *TAG = "BENCH";

static void pc_supervision_task(
    void *parameter
)
{
    (void)parameter;

    char command[64];

    ESP_LOGI(
        TAG,
        "PC commands: GET_VERSIONS | GET_REFERENCE | FORWARD_COMMAND GET_STATUS"
    );

    while (1)
    {
        if (
            fgets(
                command,
                sizeof(command),
                stdin
            ) == NULL
        )
        {
            vTaskDelay(
                pdMS_TO_TICKS(20)
            );

            continue;
        }

        command[
            strcspn(
                command,
                "\r\n"
            )
        ] = '\0';

        if (command[0] == '\0')
        {
            continue;
        }

        bench_command_t parsed =
            bench_protocol_parse_command(
                command
            );

        bench_protocol_handle_command(
            parsed
        );
    }
}

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Bench Controller started"
    );

    ESP_LOGI(
        TAG,
        "Name: %s",
        BENCH_NAME
    );

    ESP_LOGI(
        TAG,
        "Version: %s",
        BENCH_VERSION
    );

    bench_sensor_init();

    bench_uart_init();

    ESP_LOGI(
        TAG,
        "UART BENCH <-> DUT initialized"
    );

    bench_measurement_t measurement =
        bench_sensor_read();

    if (measurement.valid)
    {
        ESP_LOGI(
            TAG,
            "Reference: %.2f C | %.2f %% | %.2f hPa",
            measurement.temperature,
            measurement.humidity,
            measurement.pressure
        );
    }
    else
    {
        ESP_LOGW(
            TAG,
            "Reference sensor unavailable"
        );
    }

    uint8_t dut_state = 0;

    bool dut_ok =
        bench_get_dut_state(
            &dut_state
        );

    if (dut_ok)
    {
        ESP_LOGI(
            TAG,
            "DUT state received: %u",
            (unsigned int)dut_state
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "DUT GET_STATUS failed"
        );
    }

    dut_measurement_t dut_measurement = {0};

    bool telemetry_ok =
        bench_get_dut_measurement(
            &dut_measurement
        );

    if (
        measurement.valid &&
        telemetry_ok &&
        dut_measurement.valid
    )
    {
        bench_comparison_t comparison =
            bench_compare_measurements(
                dut_measurement.temperature,
                dut_measurement.humidity,
                dut_measurement.pressure,
                measurement.temperature,
                measurement.humidity,
                measurement.pressure
            );

        ESP_LOGI(
            TAG,
            "Delta T=%.2f C | H=%.2f %% | P=%.2f hPa",
            comparison.temperature_delta,
            comparison.humidity_delta,
            comparison.pressure_delta
        );
    }
    else
    {
        ESP_LOGW(
            TAG,
            "Comparison unavailable"
        );
    }

    bool smoke_ok =
        bench_smoke_test_run();

    if (smoke_ok)
    {
        ESP_LOGI(
            TAG,
            "Bench smoke test PASS"
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "Bench smoke test FAIL"
        );
    }

    BaseType_t task_created =
        xTaskCreate(
            pc_supervision_task,
            "PcSupervision",
            4096,
            NULL,
            4,
            NULL
        );

    if (task_created != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "PC supervision task creation failed"
        );
    }

    ESP_LOGI(
        TAG,
        "Bench initialization complete"
    );
}

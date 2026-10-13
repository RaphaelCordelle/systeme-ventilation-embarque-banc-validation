#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "bench_uart.h"
#include "bench_config.h"
#include "bench_version.h"
#include "bench_sensor.h"
#include "bench_protocol.h"
#include "bench_compare.h"
#include "bench_smoke_test.h"


static const char *TAG = "BENCH";


/*
 * Mutex utilise pour eviter que deux taches communiquent
 * avec le DUT en meme temps.
 *
 * Exemple :
 * - ComparisonTask demande une telemetrie
 * - PcSupervision demande GET_STATUS
 *
 * Sans protection, les deux echanges UART peuvent se melanger.
 */
static SemaphoreHandle_t dut_access_mutex = NULL;


/*
 * Les commandes acceptees depuis le PC.
 *
 * On garde toutes les commandes deja presentes auparavant.
 */
static const char *PC_COMMANDS[] =
{
    "GET_VERSIONS",
    "GET_REFERENCE",
    "GET_DUT_TELEMETRY",
    "GET_DIAGNOSTICS",
    "GET_SNAPSHOT",
    "AUTO_ON",
    "AUTO_OFF",
    "RUN_SMOKE",
    "FORWARD_COMMAND",
    "FORWARD_COMMAND GET_STATUS",
    "FORWARD_COMMAND OPEN",
    "FORWARD_COMMAND CLOSE"
};


#define PC_COMMAND_COUNT \
    (sizeof(PC_COMMANDS) / sizeof(PC_COMMANDS[0]))

#define PC_COMMAND_BUFFER_SIZE 96
#define PC_INPUT_CHUNK_SIZE    32

/*
 * Comparaison toujours periodique, mais toutes les 5 secondes
 * afin d'eviter de remplir le terminal en permanence.
 */
#define COMPARISON_PERIOD_MS BENCH_COMPARISON_PERIOD_MS


/*
 * Verifie si le buffer actuel peut etre le debut
 * d'au moins une commande valide.
 *
 * Exemples :
 *
 * "FORW"
 *      -> oui
 *
 * "FORWARD_COMM"
 *      -> oui
 *
 * "XYZ"
 *      -> non
 */
static bool pc_is_command_prefix(
    const char *buffer,
    size_t length
)
{
    for (
        size_t i = 0;
        i < PC_COMMAND_COUNT;
        i++
    )
    {
        size_t command_length =
            strlen(
                PC_COMMANDS[i]
            );

        if (
            length <= command_length &&
            strncmp(
                PC_COMMANDS[i],
                buffer,
                length
            ) == 0
        )
        {
            return true;
        }
    }

    return false;
}


/*
 * Verifie si le buffer correspond exactement
 * a une commande connue.
 */
static bool pc_is_exact_command(
    const char *buffer
)
{
    for (
        size_t i = 0;
        i < PC_COMMAND_COUNT;
        i++
    )
    {
        if (
            strcmp(
                buffer,
                PC_COMMANDS[i]
            ) == 0
        )
        {
            return true;
        }
    }

    return false;
}


/*
 * Certaines commandes sont elles-memes le prefixe
 * d'une commande plus longue.
 *
 * Exemple :
 *
 * "FORWARD_COMMAND"
 *
 * est valide, mais peut aussi devenir :
 *
 * "FORWARD_COMMAND OPEN"
 * "FORWARD_COMMAND CLOSE"
 * "FORWARD_COMMAND GET_STATUS"
 *
 * Donc on ne doit pas executer FORWARD_COMMAND
 * immediatement avant de savoir si la ligne continue.
 */
static bool pc_has_longer_command(
    const char *buffer
)
{
    size_t length =
        strlen(
            buffer
        );

    for (
        size_t i = 0;
        i < PC_COMMAND_COUNT;
        i++
    )
    {
        size_t command_length =
            strlen(
                PC_COMMANDS[i]
            );

        if (
            command_length > length &&
            strncmp(
                PC_COMMANDS[i],
                buffer,
                length
            ) == 0
        )
        {
            return true;
        }
    }

    return false;
}


/*
 * Execute une commande PC complete.
 */
static void pc_execute_command(
    const char *command
)
{
    bench_command_t parsed =
        bench_protocol_parse_command(
            command
        );

    if (
        parsed ==
        BENCH_CMD_UNKNOWN
    )
    {
        ESP_LOGW(
            TAG,
            "PC: unknown command [%s]",
            command
        );

        return;
    }


    /*
     * Affichage propre :
     * uniquement une commande complete,
     * jamais les fragments recus.
     */
    ESP_LOGI(
        TAG,
        "PC RX: [%s]",
        command
    );


    /*
     * Une seule tache a la fois peut parler au DUT.
     */
    if (
        dut_access_mutex != NULL
    )
    {
        if (
            xSemaphoreTake(
                dut_access_mutex,
                pdMS_TO_TICKS(1500)
            ) != pdTRUE
        )
        {
            ESP_LOGW(
                TAG,
                "PC: DUT busy - command not executed"
            );

            return;
        }
    }


    if (parsed == BENCH_CMD_RUN_SMOKE)
    {
        bool smoke_ok = bench_smoke_test_run();
        ESP_LOGI(TAG, "RUN_SMOKE -> %s", smoke_ok ? "PASS" : "FAIL");
    }
    else
    {
        bench_protocol_handle_command(
            parsed
        );
    }


    if (
        dut_access_mutex != NULL
    )
    {
        xSemaphoreGive(
            dut_access_mutex
        );
    }
}


/*
 * Recoit UN caractere du flux PC.
 *
 * Le but est de reconstruire une commande complete,
 * peu importe comment fgets() a decoupe les donnees.
 */
static void pc_feed_character(
    char c,
    char *command_buffer,
    size_t *command_length
)
{
    /*
     * Retour arriere clavier.
     */
    if (
        c == '\b' ||
        c == 0x7F
    )
    {
        if (
            *command_length > 0
        )
        {
            (*command_length)--;

            command_buffer[
                *command_length
            ] = '\0';
        }

        return;
    }


    /*
     * Fin de ligne.
     *
     * Utile notamment pour la commande courte
     * "FORWARD_COMMAND", qui est aussi le prefixe
     * de plusieurs commandes plus longues.
     */
    if (
        c == '\r' ||
        c == '\n'
    )
    {
        if (
            *command_length == 0
        )
        {
            return;
        }

        command_buffer[
            *command_length
        ] = '\0';


        if (
            pc_is_exact_command(
                command_buffer
            )
        )
        {
            pc_execute_command(
                command_buffer
            );
        }
        else
        {
            ESP_LOGW(
                TAG,
                "PC: invalid command [%s]",
                command_buffer
            );
        }


        *command_length = 0;
        command_buffer[0] = '\0';

        return;
    }


    /*
     * On ignore les caracteres de controle.
     *
     * Les commandes du protocole PC utilisent
     * uniquement des caracteres ASCII imprimables.
     */
    if (
        (unsigned char)c < 32 ||
        (unsigned char)c > 126
    )
    {
        return;
    }


    /*
     * Ignore les espaces au debut d'une ligne.
     */
    if (
        *command_length == 0 &&
        (
            c == ' ' ||
            c == '\t'
        )
    )
    {
        return;
    }


    /*
     * Protection du buffer.
     */
    if (
        *command_length >=
        PC_COMMAND_BUFFER_SIZE - 1
    )
    {
        ESP_LOGW(
            TAG,
            "PC: input buffer reset"
        );

        *command_length = 0;
        command_buffer[0] = '\0';

        return;
    }


    /*
     * Ajout du caractere au buffer courant.
     */
    command_buffer[
        *command_length
    ] = c;

    (*command_length)++;

    command_buffer[
        *command_length
    ] = '\0';


    /*
     * Si on vient de reconstruire une commande complete
     * qui n'est PAS le prefixe d'une commande plus longue,
     * on peut l'executer immediatement.
     *
     * C'est ce qui permet de gerer :
     *
     * FORWARD_COMM
     * A
     * ND GE
     * T_STATUS
     *
     * comme :
     *
     * FORWARD_COMMAND GET_STATUS
     */
    if (
        pc_is_exact_command(
            command_buffer
        ) &&
        !pc_has_longer_command(
            command_buffer
        )
    )
    {
        pc_execute_command(
            command_buffer
        );

        *command_length = 0;
        command_buffer[0] = '\0';

        return;
    }


    /*
     * Si le contenu actuel ne peut meme plus etre
     * le debut d'une commande connue, on se
     * resynchronise.
     *
     * On ne produit pas une avalanche de
     * "Unknown command".
     */
    if (
        !pc_is_command_prefix(
            command_buffer,
            *command_length
        )
    )
    {
        /*
         * Le caractere courant peut lui-meme etre
         * le debut de la prochaine commande.
         *
         * Les commandes finales commencent par
         * 'F', 'G', 'A' ou 'R'.
         */
        if (
            c == 'F' ||
            c == 'G' ||
            c == 'A' ||
            c == 'R'
        )
        {
            command_buffer[0] = c;
            command_buffer[1] = '\0';

            *command_length = 1;
        }
        else
        {
            command_buffer[0] = '\0';

            *command_length = 0;
        }
    }
}


static void pc_supervision_task(
    void *parameter
)
{
    (void)parameter;

    /*
     * input_chunk :
     * ce que fgets() nous donne reellement.
     *
     * command_buffer :
     * la commande complete que NOUS reconstruisons.
     */
    char input_chunk[
        PC_INPUT_CHUNK_SIZE
    ];

    char command_buffer[
        PC_COMMAND_BUFFER_SIZE
    ] = {0};

    size_t command_length = 0;


    ESP_LOGI(
        TAG,
        "PC supervision ready"
    );

    ESP_LOGI(
        TAG,
        "Commands: GET_VERSIONS | GET_REFERENCE | GET_DUT_TELEMETRY | GET_DIAGNOSTICS | GET_SNAPSHOT | AUTO_ON/OFF | RUN_SMOKE | FORWARD_COMMAND GET_STATUS/OPEN/CLOSE"
    );


    while (1)
    {
        /*
         * fgets peut renvoyer :
         *
         * FORWARD_COMM
         *
         * puis :
         *
         * AND GET_STATUS
         *
         * Ce n'est plus un probleme :
         * on traite le resultat comme un flux.
         */
        if (
            fgets(
                input_chunk,
                sizeof(input_chunk),
                stdin
            ) == NULL
        )
        {
            clearerr(
                stdin
            );

            vTaskDelay(
                pdMS_TO_TICKS(10)
            );

            continue;
        }


        for (
            size_t i = 0;
            input_chunk[i] != '\0';
            i++
        )
        {
            pc_feed_character(
                input_chunk[i],
                command_buffer,
                &command_length
            );
        }
    }
}


static void comparison_task(
    void *parameter
)
{
    (void)parameter;

    while (1)
    {
        bench_measurement_t reference =
            {0};

        dut_measurement_t dut =
            {0};

        bool dut_ok = false;


        /*
         * On reserve l'acces au DUT avant d'envoyer
         * une requete UART.
         *
         * Cela empeche une collision avec une
         * commande venant du PC.
         */
        if (
            dut_access_mutex != NULL &&
            xSemaphoreTake(
                dut_access_mutex,
                pdMS_TO_TICKS(1000)
            ) == pdTRUE
        )
        {
            reference =
                bench_sensor_read();

            dut_ok =
                bench_get_dut_measurement(
                    &dut
                );

            xSemaphoreGive(
                dut_access_mutex
            );
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Periodic comparison skipped: DUT busy"
            );

            vTaskDelay(
                pdMS_TO_TICKS(
                    COMPARISON_PERIOD_MS
                )
            );

            continue;
        }


        if (
            reference.valid &&
            dut_ok &&
            dut.valid
        )
        {
            bench_comparison_t comparison =
                bench_compare_measurements(
                    dut.temperature,
                    dut.humidity,
                    dut.pressure,
                    reference.temperature,
                    reference.humidity,
                    reference.pressure
                );

            ESP_LOGI(
                TAG,
                "COMPARE | REF %.2fC %.2f%% %.2fhPa | DUT %.2fC %.2f%% %.2fhPa | dT=%.2f dH=%.2f dP=%.2f",
                reference.temperature,
                reference.humidity,
                reference.pressure,
                dut.temperature,
                dut.humidity,
                dut.pressure,
                comparison.temperature_delta,
                comparison.humidity_delta,
                comparison.pressure_delta
            );
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Periodic comparison unavailable"
            );
        }


        vTaskDelay(
            pdMS_TO_TICKS(
                COMPARISON_PERIOD_MS
            )
        );
    }
}


void app_main(void)
{
    ESP_LOGI(
        TAG,
        "================================"
    );

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

    ESP_LOGI(
        TAG,
        "================================"
    );


    /*
     * Initialisation du capteur de reference.
     */
    bench_sensor_init();


    /*
     * Initialisation UART BENCH <-> DUT.
     */
    bench_uart_init();

    ESP_LOGI(
        TAG,
        "UART BENCH <-> DUT initialized"
    );


    /*
     * Premiere mesure du capteur de reference.
     */
    bench_measurement_t measurement =
        bench_sensor_read();

    if (
        measurement.valid
    )
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


    /*
     * Verification initiale de l'etat du DUT.
     */
    uint8_t dut_state = 0;

    bool dut_ok =
        bench_get_dut_state(
            &dut_state
        );

    if (
        dut_ok
    )
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


    /*
     * Premiere telemetrie et comparaison.
     */
    dut_measurement_t dut_measurement =
        {0};

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
            "Initial delta: T=%.2f C | H=%.2f %% | P=%.2f hPa",
            comparison.temperature_delta,
            comparison.humidity_delta,
            comparison.pressure_delta
        );
    }
    else
    {
        ESP_LOGW(
            TAG,
            "Initial comparison unavailable"
        );
    }


    /*
     * Smoke test autonome.
     */
    bool smoke_ok =
        bench_smoke_test_run();

    if (
        smoke_ok
    )
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


    /*
     * Creation du mutex APRES les tests initiaux.
     *
     * A partir d'ici plusieurs taches vont pouvoir
     * demander simultanement l'acces au DUT.
     */
    dut_access_mutex =
        xSemaphoreCreateMutex();

    if (
        dut_access_mutex == NULL
    )
    {
        ESP_LOGE(
            TAG,
            "DUT mutex creation failed"
        );

        return;
    }


    /*
     * Comparaison periodique BENCH / DUT.
     */
    BaseType_t comparison_created =
        xTaskCreate(
            comparison_task,
            "ComparisonTask",
            4096,
            NULL,
            4,
            NULL
        );

    if (
        comparison_created != pdPASS
    )
    {
        ESP_LOGE(
            TAG,
            "Comparison task creation failed"
        );
    }


    /*
     * Supervision depuis le PC.
     */
    BaseType_t supervision_created =
        xTaskCreate(
            pc_supervision_task,
            "PcSupervision",
            4096,
            NULL,
            4,
            NULL
        );

    if (
        supervision_created != pdPASS
    )
    {
        ESP_LOGE(
            TAG,
            "PC supervision task creation failed"
        );
    }


    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "Bench initialization complete"
    );

    ESP_LOGI(
        TAG,
        "HIL supervision ready"
    );

    ESP_LOGI(
        TAG,
        "================================"
    );
}
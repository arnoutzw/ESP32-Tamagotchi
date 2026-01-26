/**
 * @file ota_manager.c
 * @brief OTA update management implementation
 *
 * REQ-SW-034: OTA Updates with rollback support
 */

#include "ota_manager.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>

static const char *TAG = "ota_mgr";

//=============================================================================
// Private Variables
//=============================================================================

static ota_state_t s_ota_state = OTA_STATE_IDLE;
static ota_progress_cb_t s_progress_cb = NULL;
static char s_version[32] = "unknown";

//=============================================================================
// Private Functions
//=============================================================================

static bool validate_auth(httpd_req_t *req)
{
    char auth_header[128];
    esp_err_t ret = httpd_req_get_hdr_value_str(req, "X-OTA-Password", auth_header, sizeof(auth_header));

    if (ret != ESP_OK) {
        // Try Authorization header as fallback
        ret = httpd_req_get_hdr_value_str(req, "Authorization", auth_header, sizeof(auth_header));
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "No auth header provided");
            return false;
        }
    }

    // Simple password comparison
    if (strcmp(auth_header, OTA_PASSWORD) == 0) {
        return true;
    }

    ESP_LOGW(TAG, "Invalid OTA password");
    return false;
}

static void report_progress(int percent)
{
    if (s_progress_cb) {
        s_progress_cb(percent);
    }
}

//=============================================================================
// HTTP Handlers
//=============================================================================

static esp_err_t ota_upload_handler(httpd_req_t *req)
{
    // Validate authentication
    if (!validate_auth(req)) {
        httpd_resp_set_status(req, "401 Unauthorized");
        httpd_resp_send(req, "Unauthorized", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "OTA update request, content length: %d", req->content_len);

    if (req->content_len == 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "No firmware data", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    s_ota_state = OTA_STATE_RECEIVING;

    // Get next OTA partition
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "No OTA partition available");
        s_ota_state = OTA_STATE_ERROR;
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "No OTA partition", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing to partition: %s at 0x%lx",
             update_partition->label, update_partition->address);

    // Begin OTA
    esp_ota_handle_t ota_handle;
    esp_err_t ret = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(ret));
        s_ota_state = OTA_STATE_ERROR;
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "OTA begin failed", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // Receive and write firmware data
    char *buf = malloc(OTA_BUF_SIZE);
    if (buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate receive buffer");
        esp_ota_abort(ota_handle);
        s_ota_state = OTA_STATE_ERROR;
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "Memory allocation failed", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    int total_received = 0;
    int remaining = req->content_len;
    int last_percent = 0;

    while (remaining > 0) {
        int to_read = (remaining < OTA_BUF_SIZE) ? remaining : OTA_BUF_SIZE;
        int received = httpd_req_recv(req, buf, to_read);

        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                ESP_LOGW(TAG, "Receive timeout, retrying...");
                continue;
            }
            ESP_LOGE(TAG, "Receive error: %d", received);
            free(buf);
            esp_ota_abort(ota_handle);
            s_ota_state = OTA_STATE_ERROR;
            httpd_resp_set_status(req, "500 Internal Server Error");
            httpd_resp_send(req, "Receive failed", HTTPD_RESP_USE_STRLEN);
            return ESP_FAIL;
        }

        ret = esp_ota_write(ota_handle, buf, received);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed: %s", esp_err_to_name(ret));
            free(buf);
            esp_ota_abort(ota_handle);
            s_ota_state = OTA_STATE_ERROR;
            httpd_resp_set_status(req, "500 Internal Server Error");
            httpd_resp_send(req, "Write failed", HTTPD_RESP_USE_STRLEN);
            return ESP_FAIL;
        }

        total_received += received;
        remaining -= received;

        // Report progress
        int percent = (total_received * 100) / req->content_len;
        if (percent != last_percent) {
            last_percent = percent;
            report_progress(percent);
            if (percent % 10 == 0) {
                ESP_LOGI(TAG, "OTA progress: %d%%", percent);
            }
        }
    }

    free(buf);

    s_ota_state = OTA_STATE_VERIFYING;
    ESP_LOGI(TAG, "Received %d bytes, verifying...", total_received);

    // End OTA (validates the image)
    ret = esp_ota_end(ota_handle);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Firmware validation failed");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(ret));
        }
        s_ota_state = OTA_STATE_ERROR;
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "Firmware validation failed", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // Set boot partition
    ret = esp_ota_set_boot_partition(update_partition);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(ret));
        s_ota_state = OTA_STATE_ERROR;
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "Set boot partition failed", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    s_ota_state = OTA_STATE_REBOOTING;
    ESP_LOGI(TAG, "OTA successful! Rebooting in 2 seconds...");

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OTA successful, rebooting...", HTTPD_RESP_USE_STRLEN);

    // Delay before reboot to allow response to be sent
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();

    return ESP_OK;  // Never reached
}

static esp_err_t ota_status_handler(httpd_req_t *req)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_app_desc_t *app_desc = esp_app_get_description();

    char response[256];
    snprintf(response, sizeof(response),
             "{"
             "\"version\":\"%s\","
             "\"partition\":\"%s\","
             "\"state\":\"%d\""
             "}",
             app_desc->version,
             running->label,
             s_ota_state);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//=============================================================================
// Public Functions
//=============================================================================

esp_err_t ota_manager_init(void)
{
    // Get running partition info
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_app_desc_t *app_desc = esp_app_get_description();

    ESP_LOGI(TAG, "Running partition: %s", running->label);
    ESP_LOGI(TAG, "Firmware version: %s", app_desc->version);

    strncpy(s_version, app_desc->version, sizeof(s_version) - 1);

    s_ota_state = OTA_STATE_IDLE;
    return ESP_OK;
}

esp_err_t ota_manager_register_handlers(httpd_handle_t server)
{
    if (server == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // POST /ota - upload firmware
    httpd_uri_t ota_upload = {
        .uri = "/ota",
        .method = HTTP_POST,
        .handler = ota_upload_handler,
        .user_ctx = NULL
    };

    esp_err_t ret = httpd_register_uri_handler(server, &ota_upload);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register /ota handler: %s", esp_err_to_name(ret));
        return ret;
    }

    // GET /ota/status - get OTA status
    httpd_uri_t ota_status = {
        .uri = "/ota/status",
        .method = HTTP_GET,
        .handler = ota_status_handler,
        .user_ctx = NULL
    };

    ret = httpd_register_uri_handler(server, &ota_status);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register /ota/status handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "OTA handlers registered");
    return ESP_OK;
}

esp_err_t ota_manager_mark_valid(void)
{
    esp_err_t ret = esp_ota_mark_app_valid_cancel_rollback();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mark app valid: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Firmware marked as valid");
    return ESP_OK;
}

ota_state_t ota_manager_get_state(void)
{
    return s_ota_state;
}

void ota_manager_set_progress_callback(ota_progress_cb_t cb)
{
    s_progress_cb = cb;
}

const char *ota_manager_get_version(void)
{
    return s_version;
}

bool ota_manager_is_first_boot(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;

    esp_err_t ret = esp_ota_get_state_partition(running, &state);
    if (ret != ESP_OK) {
        return false;
    }

    return (state == ESP_OTA_IMG_PENDING_VERIFY);
}

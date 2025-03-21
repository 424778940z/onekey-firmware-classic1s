#include "ble.h"
#include "layout.h"
#include "oled.h"
#include "sys.h"
#include "timer.h"
#include "usart.h"

static usart_msg ble_usart_msg;
static bool get_ble_name = false;
static bool get_ble_ver = false;
static bool get_ble_hw_ver = false;
static bool get_ble_battery = false;
static bool get_ble_build_id = false;
static bool get_ble_hash = false;
static bool ble_connect = false;
static bool ble_switch = false;
static bool get_ble_switch = false;
static bool passkey_state = false;
static int ble_request_state = -1;
static uint8_t ble_response_buf[64];
static char ble_name[BLE_NAME_LEN + 1] = {0};
static char ble_ver[16] = {0};
static char ble_build_id[8] = {0};
static uint8_t ble_hash[32] = {0};
static HW_VER_t ble_hw_ver = HW_VER_INVALID;
static uint8_t ble_update_buffer[256] = {0};

trans_fifo ble_update_fifo = {.p_buf = ble_update_buffer,
                              .buf_size = sizeof(ble_update_buffer),
                              .over_pre = false,
                              .read_pos = 0,
                              .write_pos = 0,
                              .lock_pos = 0};

static uint8_t calXor(uint8_t *buf, uint32_t len) {
  uint8_t tmp = 0;
  uint32_t i;
  for (i = 0; i < len; i++) {
    tmp ^= buf[i];
  }
  return tmp;
}

static void ble_cmd_packet(uint8_t *value, uint8_t value_len) {
  uint8_t cmd[64] = {0};
  cmd[0] = 0x5a;
  cmd[1] = 0xa5;
  cmd[2] = ((value_len + 1) >> 8) & 0xff;
  cmd[3] = (value_len + 1) & 0xff;
  memcpy(cmd + 4, value, value_len);
  cmd[value_len + 4] = calXor(cmd, value_len + 4);
  ble_usart_send(cmd, value_len + 5);
}

void ble_request_info(uint8_t type) {
  uint8_t cmd[64] = {0};
  cmd[0] = type;
  cmd[1] = 0x01;
  cmd[2] = 0x01;
  ble_cmd_packet(cmd, 3);
}

void ble_request_switch_state(void) {
  uint8_t cmd[64] = {0};
  cmd[0] = BLE_CMD_ONOFF_BLE;
  cmd[1] = 0x01;
  cmd[2] = 0x07;
  ble_cmd_packet(cmd, 3);
}

void ble_ctl_onoff(void) {
  uint8_t cmd[64] = {0};
  cmd[0] = BLE_CMD_ONOFF_BLE;
  cmd[1] = 0x01;
  cmd[2] = 0x03;
  ble_cmd_packet(cmd, 0x03);
}

void change_ble_sta(uint8_t mode) {
  uint8_t cmd[64] = {0};
  cmd[0] = BLE_CMD_ONOFF_BLE;
  cmd[1] = 0x01;
  cmd[2] = mode;
  if (ble_switch != mode) {
    ble_cmd_packet(cmd, 0x03);
    ble_switch = mode;
  }
}

bool ble_get_pubkey(uint8_t *pubkey) {
  uint8_t cmd[64] = {0};
  uint8_t counter = 0;
  cmd[0] = BLE_CMD_DEVICE_PUBKEY;
  cmd[1] = BLE_PBUKEY_GET;
  ble_cmd_packet(cmd, 2);
  ble_request_state = -1;
  while (ble_request_state == -1) {
    counter++;
    delay_ms(500);
    if (counter > 5) {
      return false;
    }
    ble_cmd_packet(cmd, 2);
  }
  if (ble_request_state != 0) {
    return false;
  }
  memcpy(pubkey, ble_response_buf, 64);
  memset(ble_response_buf, 0x00, 64);
  return true;
}

bool ble_lock_pubkey(void) {
  uint8_t cmd[64] = {0};
  uint8_t counter = 0;
  cmd[0] = BLE_CMD_DEVICE_PUBKEY;
  cmd[1] = BLE_PBUKEY_LOCK;
  ble_cmd_packet(cmd, 2);
  ble_request_state = -1;
  while (ble_request_state == -1) {
    counter++;
    delay_ms(500);
    if (counter > 5) {
      return false;
    }
    ble_cmd_packet(cmd, 2);
  }
  if (ble_request_state != 0) {
    return false;
  }
  return true;
}

int ble_get_error(void) { return ble_request_state; }

bool ble_sign_msg(uint8_t *msg, uint32_t msg_len, uint8_t *sign) {
  uint8_t cmd[64] = {0};
  uint8_t counter = 0;
  cmd[0] = BLE_CMD_DEVICE_SIGN;
  memcpy(cmd + 1, msg, msg_len);
  ble_cmd_packet(cmd, msg_len + 1);
  ble_request_state = -1;
  while (ble_request_state == -1) {
    counter++;
    delay_ms(200);
    if (counter > 10) {
      return false;
    }
    ble_cmd_packet(cmd, msg_len + 1);
  }
  if (ble_request_state != 0) {
    return false;
  }
  memcpy(sign, ble_response_buf, 64);
  memset(ble_response_buf, 0x00, 64);
  return true;
}

bool ble_get_version(char **ver) {
  if (get_ble_ver) {
    *ver = ble_ver;
    return true;
  }
  ble_request_info(BLE_CMD_VER);
  uint8_t counter = 0;
  while (!get_ble_ver) {
    counter++;
    delay_ms(500);
    if (counter > 5) {
      return false;
    }
    ble_request_info(BLE_CMD_VER);
  }
  *ver = ble_ver;
  return true;
}

bool ble_hw_ver_state(void) { return get_ble_hw_ver; }

bool ble_get_hw_version(HW_VER_t *ver) {
  if (get_ble_hw_ver) {
    *ver = ble_hw_ver;
    return true;
  }
  ble_request_info(BLE_CMD_HW_VER);
  uint8_t counter = 0;
  while (!get_ble_hw_ver) {
    counter++;
    delay_ms(500);
    if (counter > 5) {
      return false;
    }
    ble_request_info(BLE_CMD_HW_VER);
  }
  *ver = ble_hw_ver;
  return true;
}

bool ble_hw_ver_is_pure(void) { return ble_hw_ver == HW_VER_V_PURE; }

bool ble_connect_state(void) { return ble_connect; }
bool ble_name_state(void) { return get_ble_name; }
bool ble_ver_state(void) { return get_ble_ver; }
bool ble_battery_state(void) { return get_ble_battery; }
bool ble_switch_state(void) { return get_ble_switch; }
bool ble_build_id_state(void) { return get_ble_build_id; }
bool ble_hash_state(void) { return get_ble_hash; }
char *ble_get_name(void) { return ble_name; }
char *ble_get_ver(void) { return ble_ver; }
char *ble_get_build_id(void) { return ble_build_id; }
uint8_t *ble_get_hash(void) { return ble_hash; }

void ble_set_switch(bool flag) { ble_switch = flag; }
bool ble_get_switch(void) { return ble_switch; }
bool ble_passkey_state(void) { return passkey_state; }

void ble_reset(void) {
  ble_power_off();
  delay_ms(10);
  ble_power_on();
  delay_ms(200);
}

void ble_update_poll(void) {
  uint32_t total_len;

  uint8_t buf[2] = {0};

  total_len = fifo_lockdata_len(&ble_update_fifo);
  if (total_len == 0) {
    return;
  }
  fifo_read_lock(&ble_update_fifo, buf, 2);

  if (buf[0] == 0x0B && buf[1] <= 100) {
    uint32_t percent = buf[1];
    if (percent == 99) {
      oledClear();
      oledDrawStringCenter(60, 30, "BLE update success", FONT_STANDARD);
      oledRefresh();
      layoutRefreshSet(true);
      delay_ms(500);
    } else {
      layoutProgress("Installing ble...", 10 * percent);
    }
    return;
  }
}

void ble_uart_poll(uint8_t *buf) {
  uint8_t passkey[17] = {0};
  static bool need_refresh = false;

  ble_usart_msg.len = (buf[2] << 8) + buf[3];
  ble_usart_msg.cmd = buf[4];
  ble_usart_msg.cmd_len = buf[5];
  ble_usart_msg.cmd_vale = buf + 6;

  switch (ble_usart_msg.cmd) {
    case BLE_CMD_CONNECT_STATE:
    case BLE_CMD_PAIR_STATE:
      if (ble_usart_msg.cmd_vale[0] == 0x01)
        ble_connect = true;
      else
        ble_connect = false;
      if (need_refresh) {
        need_refresh = false;
        passkey_state = false;
        layoutRefreshSet(true);
      }
      break;
    case BLE_CMD_PASSKEY:
      if (ble_usart_msg.cmd_len == 0x06) {
        for (int i = 0, j = 0; i < 16; i++) {
          if (i % 3 == 0) {
            passkey[i] = ble_usart_msg.cmd_vale[j++];
          } else {
            passkey[i] = ' ';
          }
        }
        passkey_state = true;
        layoutBlePasskey(passkey);
        need_refresh = true;
      }
      break;
    case BLE_CMD_BT_NAME:
      if (ble_usart_msg.cmd_len == BLE_NAME_LEN) {
        memcpy(ble_name, ble_usart_msg.cmd_vale, BLE_NAME_LEN);
        get_ble_name = true;
        layoutRefreshSet(true);
      }
      break;
    case BLE_CMD_BATTERY:
      if (ble_usart_msg.cmd_vale[0] <= 5) {
        get_ble_battery = true;
        battery_cap = ble_usart_msg.cmd_vale[0];
      }
      break;
    case BLE_CMD_VER:
      if (ble_usart_msg.cmd_len < sizeof(ble_ver) - 1) {
        memcpy(ble_ver, ble_usart_msg.cmd_vale, ble_usart_msg.cmd_len);
        get_ble_ver = true;
      }
      break;
    case BLE_CMD_ONOFF_BLE:
      get_ble_switch = true;
      if (ble_usart_msg.cmd_vale[0] == 0) {
        ble_switch = false;
      } else {
        ble_switch = true;
      }
      passkey_state = false;
      layoutRefreshSet(true);
      break;
    case BLE_CMD_DEVICE_PUBKEY:
      if (ble_usart_msg.cmd_len == 1) {
        ble_request_state = ble_usart_msg.cmd_vale[0];
      } else if (ble_usart_msg.cmd_len == 64) {
        ble_request_state = 0;
        memcpy(ble_response_buf, ble_usart_msg.cmd_vale, 64);
      }
      break;
    case BLE_CMD_DEVICE_SIGN:
      if (ble_usart_msg.cmd_len == 1) {
        ble_request_state = ble_usart_msg.cmd_vale[0];
      } else if (ble_usart_msg.cmd_len == 64) {
        ble_request_state = 0;
        memcpy(ble_response_buf, ble_usart_msg.cmd_vale, 64);
      }
      break;
    case BLE_CMD_BUILD_ID:
      if (ble_usart_msg.cmd_len == 7) {
        memcpy(ble_build_id, ble_usart_msg.cmd_vale, 7);
        get_ble_build_id = true;
      }
      break;
    case BLE_CMD_HASH:
      if (ble_usart_msg.cmd_len == 32) {
        memcpy(ble_hash, ble_usart_msg.cmd_vale, 32);
        get_ble_hash = true;
      }
      break;
    case BLE_CMD_HW_VER:
      if (ble_usart_msg.cmd_len == 2) {
        ble_hw_ver = ble_usart_msg.cmd_vale[1] << 8 | ble_usart_msg.cmd_vale[0];
        if (ble_hw_ver == HW_VER_V_1_X || ble_hw_ver == HW_VER_V_2_0 ||
            ble_hw_ver == HW_VER_V_PURE) {
          get_ble_hw_ver = true;
        }
      }
      break;
    default:
      break;
  }
}

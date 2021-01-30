#include "config_eeprom.h"

#include <CRC32.h>

#include "hal/i2c.h"

namespace config_eeprom {

static constexpr uint16_t kSettingsCrcAddress = 0x10;
static constexpr uint16_t kSettingsAddress = kSettingsCrcAddress + 4;

// i2c device address.
constexpr uint8_t kEepromDeviceAddress = 0x50;

struct ConfigPayload {
  // Acquisition channels offsets.
  int16_t offset1 = 0;
  int16_t offset2 = 0;
  // Acquisition direction flag.
  bool reverse_direction = false;
  // Reserve. Always write as 0.
  uint8_t reserved[32] = {};
};

// sizeof() = 40 as of Jan 2021.
struct ConfigPacket {
  uint32_t crc;
  ConfigPayload payload;
};

// Use this as default package value.
static const ConfigPayload kDefaultConfigPayload = {
    .offset1 = 1800,
    .offset2 = 1800,
    .reverse_direction = false,
};

static void clear_reserved(ConfigPayload* payload) {
  memset(&payload->reserved, 0x0, sizeof(payload->reserved));
}

// 'packet.reserved should be pre cleared.
static uint32_t compute_crc(const ConfigPayload& payload) {
  // This is a light class. Ok to have on stack.
  CRC32 crc;
  crc.update(&payload, 1);
  return crc.finalize();
}

bool read_bytes(uint8_t byte_address, uint8_t* bfr, uint8_t size) {
  // Command 1: Set eeprom internal address register using a dummy write.
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
      &i2c::hi2c1, kEepromDeviceAddress << 1 | 0, &byte_address, 1, 100);
  if (status != HAL_OK) {
    return false;
  }

  // Command 2: Read N bytes.
  status = HAL_I2C_Master_Receive(&i2c::hi2c1, kEepromDeviceAddress << 1 | 1,
                                  bfr, size, 100);
  if (status != HAL_OK) {
    return false;
  }
  return true;
}

bool write_bytes(uint8_t byte_address, const uint8_t* data, uint8_t data_size) {
  uint8_t bytes_written = 0;

  while (bytes_written < data_size) {
    // Writes must be in a single 8 bytes page. Determine how many
    // bytes we have to write in the next page.
    const uint8_t next_byte_address = byte_address + bytes_written;
    const uint8_t bytes_left = data_size - bytes_written;
    uint8_t bytes_in_page = 8 - (next_byte_address & 0x7);
    if (bytes_in_page > bytes_left) {
      bytes_in_page = bytes_left;
    }

    // Preper send buffer. Start address + one to eight bytes to write.
    // bytes_in_page is in the range [1, 8].
    uint8_t bfr[1 + 8];
    bfr[0] = next_byte_address;
    for (uint8_t i = 0; i < bytes_in_page; i++) {
      bfr[i + 1] = data[bytes_written++];
    }

    // Send.
    HAL_StatusTypeDef status =
        HAL_I2C_Master_Transmit(&i2c::hi2c1, kEepromDeviceAddress << 1 | 0, bfr,
                                bytes_in_page + 1, 100);

    if (status != HAL_OK) {
      last_status = "WRITE_ERROR";
      return false;
    }

    // Let the eeprom finish pending writes, if any. See Twr in the datasheet.
    delay(10);
  }

  last_status = "WRITE_OK";
  return true;
}

static void copy_settings(const ConfigPayload& payload,
                          acquisition::Settings* settings) {
  settings->offset1 = payload.offset1;
  settings->offset2 = payload.offset2;
  settings->reverse_direction = payload.reverse_direction;
}

const char* last_status = "NONE";

// Returns true if read ok. Otherwise default settings are returned.
bool read_acquisition_settings(acquisition::Settings* settings) {
  ConfigPacket packet;

  const bool read_ok = read_bytes(0, (uint8_t*)&packet, sizeof(packet));
  if (!read_ok) {
    copy_settings(kDefaultConfigPayload, settings);
    last_status = "READ-ERROR";
    return false;
  }

  const uint32_t crc = compute_crc(packet.payload);

  if (crc != packet.crc) {
    copy_settings(kDefaultConfigPayload, settings);
    last_status = "CRC_ERROR";
    return false;
  }

  copy_settings(packet.payload, settings);
  last_status = "OK";
  return true;
}

// Returns true if written ok.
bool write_acquisition_settings(const acquisition::Settings& settings) {
  ConfigPacket packet;
  // Populate payload.
  packet.payload.offset1 = settings.offset1;
  packet.payload.offset2 = settings.offset2;
  packet.payload.reverse_direction = settings.reverse_direction;
  clear_reserved(&packet.payload);

  // Compute checkscum.
  packet.crc = compute_crc(packet.payload);

  return write_bytes(0, (uint8_t*)&packet, sizeof(packet));
}

}  // namespace config_eeprom
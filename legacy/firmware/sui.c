#include "sui.h"
#include "fsm.h"
#include "gettext.h"
#include "layout2.h"
#include "messages.h"
#include "messages.pb.h"
#include "protect.h"
#include "stdint.h"
#include "util.h"

void sui_get_address_from_public_key(const uint8_t *public_key, char *address) {
  uint8_t buf[32] = {0};
  BLAKE2B_CTX ctx;
  blake2b_Init(&ctx, 32);
  blake2b_Update(&ctx, (const uint8_t *)"\x00", 1);
  blake2b_Update(&ctx, public_key, 32);
  blake2b_Final(&ctx, buf, 32);

  address[0] = '0';
  address[1] = 'x';
  data2hexaddr((const uint8_t *)buf, 32, address + 2);
}

void sui_sign_tx(const SuiSignTx *msg, const HDNode *node, SuiSignedTx *resp) {
  char address[67] = {0};
  uint8_t digest[32] = {0};

  sui_get_address_from_public_key(node->public_key + 1, address);
  // INTENT_BYTES = b'\x00\x00\x00'
  if ((msg->raw_tx.bytes[0] != 0x00) && ((msg->raw_tx.bytes[1] != 0x00)) &&
      ((msg->raw_tx.bytes[2] != 0x00))) {
    fsm_sendFailure(FailureType_Failure_DataError, "Invalid raw tx");
    layoutHome();
  }

  BLAKE2B_CTX ctx;
  blake2b_Init(&ctx, 32);
  blake2b_Update(&ctx, msg->raw_tx.bytes, msg->raw_tx.size);
  blake2b_Final(&ctx, digest, 32);

  if (!layoutBlindSign("Sui", false, NULL, address, msg->raw_tx.bytes,
                       msg->raw_tx.size, NULL, NULL, NULL, NULL, NULL, NULL)) {
    fsm_sendFailure(FailureType_Failure_ActionCancelled,
                    "Signing cancelled by user");
    layoutHome();
    return;
  }

#if EMULATOR
  ed25519_sign(digest, 32, node->private_key, resp->signature.bytes);
#else
  hdnode_sign(node, digest, 32, 0, resp->signature.bytes, NULL, NULL);
#endif
  memcpy(resp->public_key.bytes, node->public_key + 1, 32);
  resp->signature.size = 64;
  resp->public_key.size = 32;
  msg_write(MessageType_MessageType_SuiSignedTx, resp);
}

static void uleb_encode(int num, uint8_t *num_bytes, int *len) {
  while (num > 0) {
    num_bytes[*len] = num & 127;
    if (num >>= 7) {
      num_bytes[*len] |= 128;
    }
    *len += 1;
  }
}

void sui_message_sign(const SuiSignMessage *msg, const HDNode *node,
                      SuiMessageSignature *resp) {
  uint8_t digest[32] = {0};
  uint8_t num_bytes[32] = {0x3, 0x0, 0x0};  // Personal Message
  int num_bytes_len = 3;

  uleb_encode(msg->message.size, num_bytes, &num_bytes_len);

  BLAKE2B_CTX ctx;
  blake2b_Init(&ctx, 32);
  blake2b_Update(&ctx, num_bytes, num_bytes_len);
  blake2b_Update(&ctx, msg->message.bytes, msg->message.size);
  blake2b_Final(&ctx, digest, 32);
#if EMULATOR
  ed25519_sign(digest, 32, node->private_key, resp->signature.bytes);
#else
  hdnode_sign(node, digest, 32, 0, resp->signature.bytes, NULL, NULL);
#endif
  resp->signature.size = 64;
  msg_write(MessageType_MessageType_SuiMessageSignature, resp);
}

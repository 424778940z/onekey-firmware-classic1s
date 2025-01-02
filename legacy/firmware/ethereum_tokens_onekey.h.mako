// This file is automatically generated from ethereum_tokens_onekey.h.mako
// DO NOT EDIT

#ifndef __ETHEREUM_TOKENS_ONEKEY_H__
#define __ETHEREUM_TOKENS_ONEKEY_H__

#include <stdint.h>

<% erc20_list = list(erc20) %>\
#define TOKENS_COUNT ${len(erc20_list) + 9}

typedef struct {
	uint64_t chain_id;
	const char * const address;
	const char * const ticker;
	int decimals;
} TokenType;

extern const TokenType tokens[TOKENS_COUNT];

extern const TokenType *UnknownToken;

const TokenType *tokenByChainAddress(uint64_t chain_id, const uint8_t *address);

#endif

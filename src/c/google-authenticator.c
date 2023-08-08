// Helper program to generate a new secret for use in two-factor
// authentication.
//
// Copyright 2010 Google Inc.
// Author: Markus Gutschke
//
// Adapted for the Pebble Smartwatch
// Author: Kevin Cooper
// https://github.com/JumpMaster/PebbleAuth
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pebble.h"
#include "base32.h"
#include "hmac.h"
#include "sha1.h"
#include "hmac_sha256.h"
#include "sha256.h"
#include "google-authenticator.h"

char *generateCode(const char *key, const char *hash, int timezone_offset) {
	if (strcmp(hash, "SHA256") == 0) {
		return generateSha256Code(key, timezone_offset);
	}
	else {
		return generateSha1Code(key, timezone_offset);
	}
}

char *generateSha1Code(const char *key, int timezone_offset) {
	
	#ifdef PBL_SDK_2
		long tm = (time(NULL) + (timezone_offset*60))/30;
	#else
		long tm = time(NULL)/30;
	#endif
		
	uint8_t challenge[8];
	for (int i = 8; i--; tm >>= 8) {
		challenge[i] = tm;
	}
	
	// Estimated number of bytes needed to represent the decoded secret. Because
	// of white-space and separators, this is an upper bound of the real number,
	// which we later get as a return-value from base32_decode()
	int secretLen = (strlen(key) + 7)/8*BITS_PER_BASE32_CHAR;
	
	// Sanity check, that our secret will fixed into a reasonably-sized static
	// array.
	if (secretLen < 0 || secretLen > 100) {
		return "000000";
	}
	
	// Decode secret from Base32 to a binary representation, and check that we
	// have at least one byte's worth of secret data.
	uint8_t secret[100];
	if ((secretLen = base32_decode((const uint8_t *)key, secret, secretLen))<1) {
		return "000000";
	}

	// Compute the HMAC_SHA1 of the secret and the challenge.
	uint8_t hash[SHA1_DIGEST_LENGTH];
	hmac_sha1(secret, secretLen, challenge, 8, hash, SHA1_DIGEST_LENGTH);
	
	// Pick the offset where to sample our hash value for the actual verification
	// code.
	int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;
	
	// Compute the truncated hash in a byte-order independent loop.
	unsigned int truncatedHash = 0;
	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash  |= hash[offset + i];
	}
	
	// Truncate to a smaller number of digits.
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= VERIFICATION_CODE_MODULUS;

	// Convert the truncatedHash int to a Char/String
	static char tokenText[7] = "000000";

	for(int i = 5; i >= 0; i--)
	{
		tokenText[i] = '0' + (truncatedHash % 10);
		truncatedHash /= 10;
	}
	tokenText[6] = '\0';

	return tokenText;
}

char *generateSha256Code(const char *key, int timezone_offset) {
	
	#ifdef PBL_SDK_2
		long tm = (time(NULL) + (timezone_offset*60))/30;
	#else
		long tm = time(NULL)/30;
	#endif
		
	uint8_t challenge[8];
	for (int i = 8; i--; tm >>= 8) {
		challenge[i] = tm;
	}
	
	// Estimated number of bytes needed to represent the decoded secret. Because
	// of white-space and separators, this is an upper bound of the real number,
	// which we later get as a return-value from base32_decode()
	int secretLen = (strlen(key) + 7)/8*BITS_PER_BASE32_CHAR;
	
	// Sanity check, that our secret will fixed into a reasonably-sized static
	// array.
	if (secretLen < 0 || secretLen > 100) {
		return "000000";
	}
	
	// Decode secret from Base32 to a binary representation, and check that we
	// have at least one byte's worth of secret data.
	uint8_t secret[100];
	if ((secretLen = base32_decode((const uint8_t *)key, secret, secretLen))<1) {
		return "000000";
	}

	// Compute the HMAC_SHA1 of the secret and the challenge.
	uint8_t hash[SHA256_HASH_SIZE];
	hmac_sha256(secret, secretLen, challenge, 8, hash, SHA256_HASH_SIZE);
	
	// Pick the offset where to sample our hash value for the actual verification
	// code.
	int offset = hash[SHA256_HASH_SIZE - 1] & 0xF;
	
	// Compute the truncated hash in a byte-order independent loop.
	unsigned int truncatedHash = 0;
	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash  |= hash[offset + i];
	}
	
	// Truncate to a smaller number of digits.
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= VERIFICATION_CODE_MODULUS;

	// Convert the truncatedHash int to a Char/String
	static char tokenText[7] = "000000";

	for(int i = 5; i >= 0; i--)
	{
		tokenText[i] = '0' + (truncatedHash % 10);
		truncatedHash /= 10;
	}
	tokenText[6] = '\0';

	return tokenText;
}

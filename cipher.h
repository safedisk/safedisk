#pragma once

#include "slice.h"

#include <openssl/aes.h>

extern "C" {
// WTF openssl did you forget this one?
#include <openssl/modes.h>
}

class cipher_key_t : public frslice_t<cipher_key_t, 32> 
{
	using frslice_t::frslice_t;
};

class cipher_ctx_t
{
public:
	// Create an random cipher context
	cipher_ctx_t();
	// Create a new cipher context with a key
	cipher_ctx_t(const cipher_key_t& key);
	// Free cipher context
	~cipher_ctx_t();

	// Reset key
	void set_key(const cipher_key_t& key);
	// GCM encrypt + sign with a specific IV	
	slice_t encrypt_and_sign(uint64_t iv, const rslice_t& in);
	// GCM decrypt + verify with a specific IV, return false on error
	bool decrypt_and_verify(uint64_t iv, slice_t& out, const rslice_t& in);

	// Partial GCM support
	void gcm_set_iv(uint64_t iv);
	void gcm_partial_encrypt(const slice_t& out, const rslice_t& in);
	void gcm_partial_encrypt(const slice_t& io) { gcm_partial_encrypt(io, io); }
	void gcm_partial_decrypt(const slice_t& out, const rslice_t& in);
	void gcm_partial_decrypt(const slice_t& io) { gcm_partial_decrypt(io, io); }
	void gcm_finalize(const slice_t& out);

	// Simple in place OFB encryption
	void encrypt(const rslice_t& iv, const slice_t& buf);
	// Simple in place OFB decryption
	// Actually, same as encryption, but let's be nice for naming
	void decrypt(const rslice_t& iv, const slice_t& buf);
	
private:
	AES_KEY m_key;
	GCM128_CONTEXT* m_context;
};

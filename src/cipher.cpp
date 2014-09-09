#include "cipher.h"

#include <arpa/inet.h>
#include <syslog.h>
#include <assert.h>

#include <openssl/rand.h>
#include <openssl/aes.h>

extern "C" {
// WTF openssl did you forget this one?
#include <openssl/modes.h>
}

cipher_ctx_t::cipher_ctx_t()
	: m_key(make_unique<AES_KEY>())
{
	slice_t key(32);
	// Make some random bytes
	RAND_pseudo_bytes(key.ubuf(), key.size());
	// Construct an AES key schedule
	AES_set_encrypt_key(key.ubuf(), 256, m_key.get());	
	// Create a new GCM context using the AES key
	m_context = CRYPTO_gcm128_new(m_key.get(), block128_f(AES_encrypt));
}

cipher_ctx_t::cipher_ctx_t(const cipher_key_t& key)
	: m_key(make_unique<AES_KEY>())
{
	assert(key.cast().size() == 32);
	// Construct an AES key schedule
	AES_set_encrypt_key(key.cast().ubuf(), 256, m_key.get());	
	// Create a new GCM context using the AES key
	m_context = CRYPTO_gcm128_new(m_key.get(), block128_f(AES_encrypt));
}

void cipher_ctx_t::set_key(const cipher_key_t& key)
{
	assert(key.cast().size() == 32);
	// Set the AES key schedule
	AES_set_encrypt_key(key.cast().ubuf(), 256, m_key.get());	
}

cipher_ctx_t::~cipher_ctx_t()
{
	CRYPTO_gcm128_release(m_context);
}

slice_t cipher_ctx_t::encrypt_and_sign(uint64_t iv, const rslice_t& in)
{
	gcm_set_iv(iv);
	// Prepare an output buffer that is 16 bytes larger than the input
	// The first 16 bytes will be used to hold the 'tag' used by GCM for auth
	slice_t out(in.size() + 16);
	// Do the actual encrypt into byte 16+ of the new buffer
	gcm_partial_encrypt(out.slice(16, in.size()), in);
	// Dump the tag into the begining of the output buffer
	gcm_finalize(out.slice(0, 16));
	// Return result
	return out;
}

bool cipher_ctx_t::decrypt_and_verify(uint64_t iv, slice_t& out, const rslice_t& in)
{
	// Fail early if buffer too small
	if (in.size() < 16) {
		syslog(LOG_ERR, "Trying to decrypt runt packet");
		return false;
	}

	gcm_set_iv(iv);
	// Prepare an output buffer that is 16 bytes smaller than the input (minus the tag)
	out = slice_t(in.size() - 16);
	// Do the actual decrypt, skipping tag
	gcm_partial_decrypt(out, in.slice(16, out.size()));
	// Get tag for comparision
	slice_t tag(16);
	gcm_finalize(tag);
	// Check tag
	if (memcmp(tag.buf(), in.buf(), 16) != 0) {
		syslog(LOG_ERR, "Broken tag on incoming packet");
		return false;
	}
	// Return result
	return true;
}

void cipher_ctx_t::gcm_set_iv(uint64_t iv)
{
	// Make a 12 byte iv that hold a network order version of the IV
	// NOTE: 12 bytes is a 'magic' IV size for GCM that has better performance
	byte iv_buf[12];
	uint32_t* iv_ul = (uint32_t*) iv_buf;
	iv_ul[0] = 0;
	iv_ul[1] = htonl(iv >> 32);
	iv_ul[2] = htonl(iv & 0xffffffff);
	// Set the IV	
	CRYPTO_gcm128_setiv(m_context, iv_buf, 12);
}

void cipher_ctx_t::gcm_partial_encrypt(const slice_t& out, const rslice_t& in)
{
	assert(out.size() == in.size());
	CRYPTO_gcm128_encrypt(m_context, in.ubuf(), out.ubuf(), in.size());
}

void cipher_ctx_t::gcm_partial_decrypt(const slice_t& out, const rslice_t& in)
{
	assert(out.size() == in.size());
	CRYPTO_gcm128_decrypt(m_context, in.ubuf(), out.ubuf(), in.size());
}

void cipher_ctx_t::gcm_finalize(const slice_t& out)
{
	assert(out.size() == 16);
	CRYPTO_gcm128_tag(m_context, out.ubuf(), 16);
}

void cipher_ctx_t::encrypt(const rslice_t& iv, const slice_t& buf) {
	assert(iv.size() >= AES_BLOCK_SIZE);
	byte ivb[AES_BLOCK_SIZE];
	memcpy(ivb, iv.buf(), AES_BLOCK_SIZE);
	int num = 0;
	AES_ofb128_encrypt(buf.ubuf(), buf.ubuf(), buf.size(), m_key.get(), ivb, &num);
}

void cipher_ctx_t::decrypt(const rslice_t& iv, const slice_t& buf) {
	return encrypt(iv, buf);
}


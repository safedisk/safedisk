
#include "digest.h"
#include <openssl/sha.h>

digest_t compute_digest(const rslice_t& data)
{
	slice_t out(SHA256_DIGEST_LENGTH);
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, data.buf(), data.size());
	SHA256_Final(out.ubuf(), &ctx);
	return out;
}


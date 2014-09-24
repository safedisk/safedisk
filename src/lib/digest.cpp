/*  Safedisk
 *  Copyright (C) 2014  Jeremy Bruestle
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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


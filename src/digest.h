
#pragma once

#include "types.h"
#include "slice.h"

// SHA-256, hardcode length so we don't have to include openssl here
class digest_t : public frslice_t<digest_t, 32> {
	using frslice_t::frslice_t;
};

digest_t compute_digest(const rslice_t& data);


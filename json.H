#pragma once
#include <pd/pi/pi.H>
#include <pd/base/string.H>
#include <pd/pi/pi_pro.H>

const pd::pi_t::root_t *pibf_parse(pd::in_segment_t &in);

pd::pi_t const *yabs_pi_object_lookup(pd::pi_t const *obj, char const *find);

static inline size_t pi_get_buf_size(size_t name_size)
{
	return sizeof(pd::pi_t) * (
		pd::pi_t::root_t::_size() + pd::pi_t::string_t::_size(name_size)
	);
}

struct mem_buf_t : public pd::pi_t::mem_t {

	virtual void *alloc(size_t _size) const {
		return malloc(_size);
	}

	virtual void free(void *ptr) const { 
		free(ptr);
	}

	inline mem_buf_t() throw()
	{ }

	inline ~mem_buf_t() throw() { }
};

char const *pi_get_string(pd::pi_t const *obj, size_t *len);

uint64_t pi_get_int(pd::pi_t const *root, char const *name, uint64_t defval);

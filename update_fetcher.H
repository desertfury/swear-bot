#pragma once

#include <unistd.h>
#include <pd/base/stat_ctx.H>

#include <pd/base/string.H>
#include <pd/base/time.H>
#include <pd/base/size.H>

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

#include <pd/bq/bq_util.H>
#include <pd/bq/bq_cond.H>

#include "handler_swear.H"

#include <atomic>

namespace phantom {

typedef io_stream::proto_http::handler_swear_bot::conf_t conf_t;
typedef io_stream::proto_http::handler_swear_bot::ext_t ext_t;
typedef io_stream::proto_http::handler_swear_bot::ext_res_t ext_res_t;

class io_update_fetcher_t : public io_t {
public:
	struct config_t;
	io_update_fetcher_t(const string_t& name, const config_t& config);
	~io_update_fetcher_t();

	void stat_print() const {}

	conf_t const &conf;
	interval_t delta;
	ext_t const *api_ext;
	mutable size_t last_update_id;
	std::atomic_bool active;

private:

	//! io_t method implementations
	void init();
	void run() const;
	void fini();
};

struct io_update_fetcher_t::config_t : public io_t::config_t {
	// The update_fetcher object.
	config_binding_type_ref(conf_t);
	config::objptr_t<conf_t> conf;
	interval_t delta;

	inline config_t()
	{}
	~config_t() throw() {}

	void check(const in_t::ptr_t& p) const;
};

} // namespace phantom

#pragma once

#include <phantom/io_stream/proto_http/handler.H>
#include <phantom/module.H>

#include <pd/bq/bq_conn_fd.H>
#include <pd/bq/bq_out.H>
#include <pd/bq/bq_in.H>
#include <pd/bq/bq_util.H>

#include <pd/base/exception.H>
#include <pd/base/config_switch.H>

#include "ext.H"

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_swear_bot {

struct conf_t {
	struct config_t;

	typedef handler_swear_bot::ext_t ext_t;
	typedef handler_swear_bot::logger_ext_t logger_ext_t;

	interval_t ext_block_timeout;

	size_t exts_count;
	ext_t **exts;

	logger_ext_t *logger_ext;

	ext_t const *lookup(str_t tag) const;

	conf_t(string_t const &, config_t const &config);
	~conf_t() throw();
};

}

MODULE(swear_bot);

class handler_swear_bot_t : public handler_t {

	virtual void do_proc(request_t const &request, reply_t &reply) const;

public:
	typedef handler_swear_bot::conf_t conf_t;

	conf_t const &conf;
	handler_swear_bot::ext_t const *api_ext;

	struct config_t : handler_t::config_t {
		config_binding_type_ref(conf_t);
		config::objptr_t<conf_t> conf;

		inline config_t() throw() :
			handler_t::config_t(), conf() { }

		inline ~config_t() throw() { }

		void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);

			if(!conf)
				config::error(ptr, "conf is required");
		}
	};

	inline handler_swear_bot_t(string_t const &name, config_t const &config):
		handler_t(name, config), conf(*config.conf), api_ext(nullptr)
	{
		api_ext = conf.lookup(CSTR("api"));

		if(!api_ext)
			throw exception_log_t(log::error, "Api ext is required");
	}
	
	handler_swear_bot::ext_req_t *get_updates() const;

	inline ~handler_swear_bot_t() throw() { }
};

} } }


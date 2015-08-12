#include "handler_swear.H"
#include "content.H"

namespace phantom { namespace io_stream { namespace proto_http {

using namespace pd;

namespace handler_swear_bot {

struct conf_t::config_t {
	interval_t ext_block_timeout;

	typedef config::switch_t<string_t, config::struct_t<ext_t::config_t>, string_t::cmp<lower_t> > ext_switch_t;
	ext_switch_t exts;

	config_binding_type_ref(logger_ext_t);
	config::objptr_t<logger_ext_t> logger_ext;

	inline config_t() :
		ext_block_timeout(interval::second) { }

	inline ~config_t() throw() { }

	void check(in_t::ptr_t const &ptr) const {
		for(ext_switch_t::ptr_t lptr = exts; lptr; ++lptr) {
			typedef config::list_t<config::objptr_t<ext_t::proto_t> > clients_list_t;

			ext_t::config_t const &ext_config = lptr.val();

			if(!ext_config.hostname)
				config::error(ptr, "ext.hostname is required");
		}
	}

};

namespace conf {
config_binding_sname(conf_t);
config_binding_value(conf_t, ext_block_timeout);
config_binding_value(conf_t, exts);
config_binding_type(conf_t, logger_ext_t);
config_binding_value(conf_t, logger_ext);
config_binding_ctor_(conf_t);
}

conf_t::conf_t(string_t const &, config_t const &config) :
	ext_block_timeout(config.ext_block_timeout),
	exts_count(0), exts(NULL), logger_ext(config.logger_ext) {

	for(config_t::ext_switch_t::ptr_t p = config.exts; p; ++p)
		++exts_count;

	if(exts_count) {
		exts = new ext_t *[exts_count];

		size_t i = 0;
		try {
			for(config_t::ext_switch_t::ptr_t p = config.exts; p; ++p)
				exts[i++] = new ext_t(p.key(), p.val());
		}
		catch(...) {
			while(i)
				delete exts[--i];

			delete [] exts;

			throw;
		}
	}
}

ext_t const *conf_t::lookup(str_t tag) const {
	size_t il = 0, ih = exts_count;
	while(il < ih) {
		size_t i = (il + ih) / 2;
		cmp_t res = string_t::cmp<lower_t>(exts[i]->tag, tag);

		if(res) return exts[i];
		if(res.is_greater()) ih = i;
		else il = i + 1;
	}

	return NULL;
}

conf_t::~conf_t() throw() {
	if(exts_count) {
		size_t i = exts_count;

		while(i)
			delete exts[--i];

		delete [] exts;
	}
}

} // namespace handler_swear_bot

handler_swear_bot::ext_req_t *handler_swear_bot_t::get_updates() const {
	return api_ext->mkreq(STRING("getUpdates"), string_t::empty, conf.logger_ext);
}


void handler_swear_bot_t::do_proc(request_t const &, reply_t &reply) const {
	auto *req = get_updates();
	if (req) {
		auto res = req->get_res();
		handler_swear_bot::content_t *content = new handler_swear_bot::content_t(res.content);
		reply.set(content);
		delete req;
		return;
	}
	handler_swear_bot::content_t *content = new handler_swear_bot::content_t();
	reply.set(content);
}

namespace handler_swear_bot {
config_binding_sname(handler_swear_bot_t);
config_binding_type(handler_swear_bot_t, conf_t);
config_binding_value(handler_swear_bot_t, conf);
config_binding_parent(handler_swear_bot_t, handler_t);
config_binding_ctor(handler_t, handler_swear_bot_t);
}


}}} // namespace phantom::io_stream::proto_http

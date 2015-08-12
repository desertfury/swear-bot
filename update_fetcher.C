#include "update_fetcher.H"

#include <phantom/scheduler.H>
#include <pd/base/log.H>
#include <pd/bq/bq_job.H>

#include <limits>
#include <string.h>

#include "json.H"
#include "swear.H"

namespace phantom {

// Config implementation
void io_update_fetcher_t::config_t::check(const in_t::ptr_t& p) const {
	io_t::config_t::check(p);

	if(!conf) {
		config::error(p, "update_fetcher_t object is required");
	}

	if (delta < interval::millisecond) {
		config::error(p, "non-zero delta value is required");
	}
}

void io_update_fetcher_t::init() {
	active.store(true);
}

void io_update_fetcher_t::fini() {
	active.store(false);
}

void io_update_fetcher_t::run() const {
//	update_fetcher->run(name);
//	size_t bq_number = scheduler.bq_n();

//	for(size_t i = 0; i < bq_number; ++i) {
//		bq_job(&update_fetcher_type::loop)(*update_fetcher)->run(scheduler.bq_thr(i));
//	}

//	struct reply_t {
//		int64_t chat_id;
//		char const *message_text;
//		size_t message_len;
//	};

	while (active.load() == true) {

		interval_t t = delta;
		if(bq_sleep(&t) < 0)
			throw exception_sys_t(log::error, errno, "bq_sleep: %m");

		string_t::ctor_t path_ctor(strlen("getUpdates?offset=") + 21);
		path_ctor.print(CSTR("getUpdates?offset=")).print(last_update_id + 1);
		auto req = api_ext->mkreq(string_t(path_ctor), string_t::empty, conf.logger_ext);
		if (!req) {
			log_error("cannot make request");
			continue;
		}
		auto res = req->get_res();
		if (res.err != ext_res_t::ext_ok) {
			delete req;
			continue;
		}
		auto root = pibf_parse(res.content);
		if (!root) {
			log_error("not a json");
			delete req;
			continue;
		}
		auto result_array_obj = yabs_pi_object_lookup(&root->value, "result");
		if (!result_array_obj || result_array_obj->type() != pd::pi_t::_array) {
			log_error("not an array_object on reslut path");
			delete req;
			continue;
		}
		auto *array = &(result_array_obj->__array());
		for (size_t i = 0; i < array->_count(); ++i) {
			auto res_obj = &((*array)[i]);
			auto text_obj = yabs_pi_object_lookup(res_obj, "message.text");
			size_t length = 0;
			char const *text = pi_get_string(text_obj, &length);

			if (length > strlen("/get")) {
				if (!memcmp(text, "/get", 4)) {
					text += 4;
					length -= 4;
				}
			}

			auto new_update_id = pi_get_int(res_obj, "update_id", 0);
			if (new_update_id > last_update_id)
				last_update_id = new_update_id;

			auto chat_id = pi_get_int(res_obj, "message.chat.id", 0);

			string_t::ctor_t ans_ctor(strlen("sendMessage?chat_id=&text=") + 21 + 21 + (length + 1) * 3);
			ans_ctor.print(CSTR("sendMessage?chat_id=")).print(chat_id).print(CSTR("&text="));

			char const *ptr = text;
			size_t new_len = length;
			bool first = true;
			while (ptr < text + length) {
				char *word = nullptr;
				size_t word_len = 0;
				get_next_word(ptr, new_len, &word, &word_len);
				if (!word || !word_len) break;
				if (!first) {
					ans_ctor.print(CSTR(" "));
				}
				int off = -1;
				char *suff = nullptr;
				get_swear_word(word, word_len, &suff, &off);
				if (off < 0 || !suff) {
					ans_ctor.print(str_t(word, word_len));
				} else {
					size_t suff_len = word_len - (suff - word);
					char const *vow = lower_vowels[off];
					ans_ctor.print(CSTR("ху")).print(str_t(vow, 2)).print(str_t(suff, suff_len));
				}
				ptr = word + word_len;
				new_len = length - (ptr - text);
				first = false;
			}
			auto send_req = api_ext->mkreq(string_t(ans_ctor), string_t::empty, conf.logger_ext);
			if (send_req) {
				req->get_res();
				delete send_req;
			}
		}
		delete req;
	}
}

io_update_fetcher_t::io_update_fetcher_t(const string_t& name, const config_t& config)
	: io_t(name, config), conf(*config.conf), delta(config.delta), api_ext(nullptr), last_update_id(0), active()
{
	api_ext = conf.lookup(CSTR("api"));

	if(!api_ext)
		throw exception_log_t(log::error, "Api ext is required");
	active.store(true);
}

io_update_fetcher_t::~io_update_fetcher_t() {}

namespace io_update_fetcher {
config_binding_sname(io_update_fetcher_t);
config_binding_parent(io_update_fetcher_t, io_t);
config_binding_value(io_update_fetcher_t, conf);
config_binding_value(io_update_fetcher_t, delta);
config_binding_ctor(io_t, io_update_fetcher_t);
} // namespace io_update_fetcher

} // namespace phantom

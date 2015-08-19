#pragma once

#include <phantom/io_stream/proto_http/handler.H>
#include <phantom/module.H>
#include "json.H"

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_swear_bot {

class content_t : public handler_t::reply_t::content_t {

	virtual http::code_t code() const throw() {
		return http::code_200;
	}
	virtual void print_header(out_t &, http::server_t const &) const { }
	virtual ssize_t size() const {
		return -1;
	}
	virtual bool print(out_t &out) const {
		out.print(str_t(res.ptr(), res.size()));
		in_segment_t in = res;
//		auto root = pibf_parse(in);
//		if (root) {
//			auto obj = pi_object_lookup(&root->value, "result[1].message.text");
//			if (obj) {
//				size_t len = 0;
//				char const *text = pi_get_string(obj, &len);
//				out.print(str_t(text, len));
//			}
//		}
		out.crlf();
	}
	pd::string_t res;
public:
	inline content_t() throw() : res() {}
	inline content_t(const in_segment_t &in) throw() : res(in) {}
	virtual ~content_t() throw() {}
};

} } } }

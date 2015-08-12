#pragma once

#include <phantom/io_client/proto_none/proto_none.H>
#include <phantom/io_client/proto_none/task.H>
#include <phantom/io_logger_file.H>

#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>
#include <pd/base/config_struct.H>
#include <pd/base/netaddr_ipv4.H>
#include <pd/base/mutex.H>

#include <pd/http/client.H>

#include <sys/time.h>

#include "content.H"

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_swear_bot {

class logger_ext_t : public io_logger_file_t {
public:
	bool log_all;

	void commit(string_t const &req, in_segment_t const &reply);

	struct config_t : io_logger_file_t::config_t {
		config::enum_t<bool> log_all;

		inline config_t() throw() :
			io_logger_file_t::config_t(), log_all(false) { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &p) const {
			io_logger_file_t::config_t::check(p);
		}
	};

	inline logger_ext_t(string_t const &name, config_t const &config) :
		io_logger_file_t(name, config), log_all(config.log_all) { }

	inline ~logger_ext_t() throw() { }
};

struct ext_req_t;

struct ext_t {
	typedef io_client::proto_none::task_t task_t;
	typedef ref_t<task_t> task_ref_t;

	typedef io_client::proto_none_t proto_t;

	proto_t &proto;

	string_t tag;
	string_t hostname, path;
	interval_t timeout;

	size_t last_update;

	struct config_t;

	ext_t(string_t const &_tag, config_t const &config);

	ext_req_t *mkreq(string_t const &path,
		string_t const &entity, logger_ext_t *logger
	) const;

	~ext_t() throw();
};

struct ext_t::config_t {
	config_binding_type_ref(proto_t);

	config::objptr_t<proto_t> proto;
	string_t hostname;
	string_t path;
	interval_t timeout;

	inline config_t() throw() :
		hostname(), path(),
		timeout(100 * interval::millisecond) { }

	void check(in_t::ptr_t const &ptr) const {
		if(!proto)
			config::error(ptr, "proto is required");
	}

	inline ~config_t() throw() { }
};

struct ext_res_t {
	in_segment_t content;
	enum ext_code_t { ext_ok = 0, ext_timeout, ext_proto_err, err_bad_req, err_not_found };
	ext_code_t err;
	ext_res_t() : content(), err(ext_ok) { }
};

class task_data_t : public ext_t::task_t {
	logger_ext_t *logger;
	bool keepalive;

	string_t spath_and_args;
	string_t headers;
	string_t entity;
	string_t entity_type;
	string_t user_agent;

	http::limits_t reply_limits;
	mutex_t reply_mutex;
	http::remote_reply_t reply;
public:
	ext_t const &ext;
	bool log_flag;

	virtual bool parse(in_t::ptr_t &ptr);
	virtual void print(out_t &out) const;
	virtual void clear();

	inline task_data_t(
		ext_t const &_ext, string_t const &_path, string_t const &_entity,
		logger_ext_t *_logger, bool _keepalive
	) :
		logger(_logger), keepalive(_keepalive),
		spath_and_args(_path),
		headers(),
		entity(_entity),
		entity_type(),
		user_agent(),
		reply_limits(1024, 128, 8 * sizeval::kilo, 8 * sizeval::mega),
		reply(),
		ext(_ext),
		log_flag(true) { }
	virtual ~task_data_t() throw() { }
	ext_res_t get_res(bool &success);
	inline in_segment_t header_lookup(str_t const &key) {
		mutex_guard_t guard(reply_mutex);
		in_segment_t const *val = reply.header.lookup(key);
		return val ? *val : string_t::empty;
	}
	void log();
	string_t request_header();
};

struct ext_req_t {
	task_data_t *task_data;
	ext_t::task_ref_t task_ref;

	ext_req_t(task_data_t *_task_data) : task_data(_task_data), task_ref(_task_data) { }

	ext_res_t get_res();
	inline void log() {
		task_data->log_flag = true;
	}
	inline ~ext_req_t() {
		task_data->log();
	}
};


}}}} // namespace phantom::io_stream::proto_http::handler_swear_bot

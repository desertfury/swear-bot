#include "ext.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_conn_fd.H>
#include <pd/bq/bq_in.H>
#include <pd/bq/bq_out.H>
#include <pd/bq/bq_util.H>

#include <pd/base/size.H>
#include <pd/base/fd_guard.H>
#include <pd/base/fd_tcp.H>
#include <pd/base/random.H>
#include <pd/base/exception.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_swear_bot {

namespace logger_ext {
config_binding_sname(logger_ext_t);
config_binding_value(logger_ext_t, log_all);
config_binding_parent(logger_ext_t, io_logger_file_t);
config_binding_ctor_(logger_ext_t);
}

void logger_ext_t::commit(string_t const &req, in_segment_t const &reply) {
	char header[1024];
	size_t header_len = ({
		out_t out(header, sizeof(header));
		out
			.print(req.size())('\t').print(reply.size())('\t')
			.print(timeval::current(), "+dz.").lf();
		out.used();
	});

	size_t iovec_max_cnt = 1 + 1 + 1 + reply.fill(NULL) + 1;

	iovec iov[iovec_max_cnt];
	size_t cnt = 0;

	iov[cnt++] = (iovec){ (void *)header, header_len };
	iov[cnt++] = (iovec){ (void *)req.ptr(), req.size() };
	iov[cnt++] = (iovec){ (void *)"\n", 1 };
	cnt += reply.fill(iov + cnt);
	iov[cnt++] = (iovec){ (void *)"\n", 1 };

	assert(cnt == iovec_max_cnt);
	writev(iov, cnt);
}

namespace ext {
config_binding_sname(ext_t);
config_binding_value(ext_t, proto);
config_binding_value(ext_t, hostname);
config_binding_value(ext_t, path);
config_binding_value(ext_t, timeout);
config_binding_ctor_(ext_t);
}

ext_t::ext_t(string_t const &_tag, config_t const &config) :
	proto(*config.proto),
	tag(_tag),hostname(config.hostname), path(config.path),
	timeout(config.timeout), last_update(0) { }

ext_t::~ext_t() throw() { }

void task_data_t::print(out_t &out) const {
	if(entity)
		out(CSTR("POST "));
	else
		out(CSTR("GET "));

	if(ext.path) out(ext.path);
	else
		if(!spath_and_args || *spath_and_args.ptr() != '/') out('/');

	if(spath_and_args) out(spath_and_args);

	out(CSTR(" HTTP/1.1")).crlf();
	out(CSTR("Host: "))(ext.hostname).crlf();

	if(user_agent)
		out(CSTR("User-Agent: "))(user_agent).crlf();

	if(!keepalive)
		out(CSTR("Connection: close")).crlf();
	else
		out(CSTR("Connection: keep-alive")).crlf();

	if(entity) {
		out(CSTR("Content-Length: ")).print(entity.size()).crlf();
		if(entity_type)
			out(CSTR("Content-Type: "))(entity_type).crlf();
	}

	if(headers)
		out(headers).crlf();

	out.crlf();

	if(entity)
		out(entity);
}

void task_data_t::clear() {
	mutex_guard_t guard(reply_mutex);
	reply.clear();
}

bool task_data_t::parse(in_t::ptr_t &ptr) {
	bool res = false;

	http::code_t code = http::code_500;

	try {
		mutex_guard_t guard(reply_mutex);

		res = reply.parse(ptr, reply_limits, /* header_only = */ false);
		code = reply.code;
	}
	catch(http::exception_t const &ex) {
		throw exception_sys_t(log::error, EIDRM, "%s", ex.msg().ptr());
	}

	if(code >= http::code_500)
		throw exception_log_t(log::error, "Got reply code %u from %.*s", reply.code, (int)ext.tag.size(), ext.tag.ptr());

	if(code != http::code_200 && code != http::code_302) {
		log_error("Got reply code %u from %.*s", reply.code, (int)ext.tag.size(), ext.tag.ptr());
	}
	else {
		log_flag = false;
	}

	return res;
}

ext_res_t task_data_t::get_res(bool &success) {
	ext_res_t res;

	http::code_t code = http::code_500;
	in_segment_t entity;

	{
		mutex_guard_t guard(reply_mutex);
		code = reply.code;
		entity = reply.entity;
	}

	if(code != http::code_200 && code != http::code_302) {
		if (code == http::code_404) {
			res.err = ext_res_t::err_not_found;
			return res;
		}
		res.err = ext_res_t::ext_proto_err;
		return res;
	}

	success = true;
	res.content = entity;
	return res;
}

void task_data_t::log() {
	if(logger && (log_flag || logger->log_all)) {
		string_t req = ({
			size_t req_len_approx =
				1024 + ext.path.size() + spath_and_args.size() + ext.hostname.size() +
				user_agent.size() + headers.size() + entity.size()
			;

			string_t::ctor_t ctor(req_len_approx);
			print(ctor);
			(string_t)ctor;
		});

		in_segment_t _reply = ({
			mutex_guard_t guard(reply_mutex);
			reply.all;
		});

		logger->commit(req, _reply);
	}
}

string_t task_data_t::request_header() {
	string_t::ctor_t ctor(spath_and_args.size() + 1);
	ctor.print(CSTR(" "))
		.print(str_t(spath_and_args.ptr(), spath_and_args.size()));
	return string_t(ctor);
}

ext_res_t ext_req_t::get_res() {
	bool success = false;
	ext_res_t res;

	interval_t s = task_data->ext.timeout;

	interval_t wtime = interval::zero;

	if(task_ref->wait(&s, &wtime)) {
		res = task_data->get_res(success);
	} else {
		task_ref->cancel();
		res.err = ext_res_t::ext_timeout;
	}

	return res;
}

ext_req_t *ext_t::mkreq(string_t const &path, string_t const &entity, logger_ext_t *logger) const
{
	ext_req_t *req = new ext_req_t(new task_data_t(*this, path, entity, logger, true));

	proto.put_task(req->task_ref);

	return req;
}

}}}} // namespace phantom::io_stream::proto_http::handler_swear_bot


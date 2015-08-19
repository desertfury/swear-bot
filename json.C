#include "json.H"

static mem_buf_t parse_mem;

const pd::pi_t::root_t *pibf_parse(pd::in_segment_t &in_seg)
{
	pd::in_t::ptr_t in = in_seg;
	try {
		return pd::pi_t::parse_text(in, parse_mem);
	}
	catch (...) {
		return nullptr;
	}
}

const pd::pi_t &pi_name_buf(const char *name, size_t name_size, char *buf, size_t buf_size)
{
	pd::str_t name_str(name, name_size);
	pd::pi_t::pro_t name_pro(name_str);
	pd::pi_t::root_t *root = pd::pi_t::build(name_pro, parse_mem);
	return root->value;
}

pd::pi_t const *pi_object_lookup(pd::pi_t const *obj, char const *find)
{
	if (!find || !(*find))
		return obj;

	if (obj) {
		if (obj->type() == pd::pi_t::_array) {
			pd::pi_t::array_t const &array = obj->__array();

			if (*find == '[') {
				char const *close = strchr(find, ']');
				errno = 0;
				if (close > find) {
					char *ptr = nullptr;
					long index = strtol(find + 1, &ptr, 10);
					if (errno == 0 && ptr == close) {
						if (array._count() > index) {
							pd::pi_t const *val = &(array[index]);
							return pi_object_lookup(val, close + 1);
						}
					}
				}
			}
		}
		else if (obj->type() == pd::pi_t::_map) {
			pd::pi_t::map_t const &map = obj->__map();

			char const *name = find;
			char const *dot = strchr(find, '.');
			char const *subs = strchr(find, '[');
			char const *part = ((dot && subs > dot) || !subs) ? dot : subs;
			size_t name_size = (part >= find) ? (size_t)(part - find) : strlen(find);

			if (dot || part != find) {
				pd::pi_t const *val = obj;

				if (name_size) {
					size_t buf_size = pi_get_buf_size(name_size);
					char buf[buf_size];
					val = map.lookup(pi_name_buf(name, name_size, buf, buf_size));
				}

				return pi_object_lookup(val, (part ? (part == dot ? (part + 1) : part) : nullptr));
			}
		}
	}
	return nullptr;
}

char const *pi_get_string(pd::pi_t const *obj, size_t *len)
{
	if (!obj || obj->type() != pd::pi_t::_string) return NULL;
	pd::pi_t::string_t const &string = obj->__string();
	if (!string._count()) return NULL;
	if (len) *len = string.str().size();
	return string.str().ptr();
}

static uint64_t pi_check_int(pd::pi_t const *obj, uint64_t defval)
{
	if (!obj) return defval;

	if (obj->type() == pd::pi_t::_int29)
		return obj->__int29();

	if (obj->type() == pd::pi_t::_uint64)
		return obj->__uint64();

	if (obj->type() == pd::pi_t::_string) {
		pd::pi_t::string_t const &string = obj->__string();

		if (string._count()) {
			pd::str_t str = string.str();
			char *ptr;
			uint64_t val = strtoul(str.ptr(), &ptr, 10);
			if (ptr == str.ptr() + str.size())
				return val;
		}
	}

	return defval;
}

uint64_t pi_get_int(pd::pi_t const *root, char const *name, uint64_t defval)
{
	pd::pi_t const *obj = pi_object_lookup(root, name);

	return pi_check_int(obj, defval);
}



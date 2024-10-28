#include <http_parser.hpp>

namespace httpparser {

	void Method::version(std::string& strVersion, unsigned long& nVersion) {
		strVersion.clear();
		nVersion = 0;
		nVersion = http_parser_version();
		unsigned int major = (nVersion >> 16) & 255;
		unsigned int minor = (nVersion >> 8) & 255;
		unsigned int patch = nVersion & 255;
		strVersion.resize(128, 0x00);
		int sprintf_size = ::sprintf_s(&strVersion[0], strVersion.size(), "%u.%u.%u", major, minor, patch);
		strVersion.resize(sprintf_size);
	}
	bool Method::parse_http_url(const std::string& url, HttpUrlFields& out) {
		bool result = false;
		out.clear();
		do {
			if (url.empty())
				break;
			http_parser_url u = { 0 };
			auto rv = http_parser_parse_url(url.data(), url.size(), 0, &u);
			if (rv < 0)
				break;
			out.port(u.port <= 0 ? 80 : u.port);
			for (unsigned int i = 0; i < http_parser_url_fields::UF_MAX; i++) {
				if ((u.field_set & (1 << i)) == 0)
					continue;
#if 0
				printf("\tfield_data[%u]: off: %u len: %u part: \"%.*s\n\"",
					i,
					u.field_data[i].off,
					u.field_data[i].len,
					u.field_data[i].len,
					url.data() + u.field_data[i].off);
#endif
				out.push(EnHttpUrlFieldType(i), url.substr(u.field_data[i].off, u.field_data[i].len));
			}
			result = !out.empty();
		} while (0);
		return result;
	}
	bool Method::parse_http_request(const std::string& http_data, size_t& nparsed, HttpRequest& out_parse_s) {
		bool result = false;
		nparsed = 0;
		out_parse_s.HeaderClear();
		do {
			if (http_data.empty())
				break;
			http_parser handle = { 0 };
			handle.data = &out_parse_s;
			http_parser_init(&handle, http_parser_type::HTTP_REQUEST);
			http_parser_settings settings = { 0 };
			settings.on_message_begin =
				[](http_parser* p)->int {
				return 0;
			};
			settings.on_url =
				[](http_parser* p, const char* data, size_t len)->int {
				return 0;
			};
			settings.on_status =
				[](http_parser* p, const char* data, size_t len)->int {
				return 0;
			};
			settings.on_header_field =
				[](http_parser* p, const char* data, size_t len)->int {
				if (p->data) {
					((HttpRequest*)p->data)->PushHeaderKey(data, len);
				}
				return 0;
			};
			settings.on_header_value =
				[](http_parser* p, const char* data, size_t len)->int {
				if (p->data) {
					((HttpRequest*)p->data)->PushHeaderValue(data, len);
				}
				return 0;
			};
			settings.on_headers_complete =
				[](http_parser* p)->int {
				return 0;
			};
			settings.on_body =
				[](http_parser* p, const char* data, size_t len)->int {
				return 0;
			};
			settings.on_message_complete =
				[](http_parser* p)->int {
				return 0;
			};
			settings.on_chunk_header =
				[](http_parser* p)->int {
				return 0;
			};
			settings.on_chunk_complete =
				[](http_parser* p)->int {
				return 0;
			};
			nparsed = http_parser_execute(&handle, &settings, http_data.data(), http_data.size());
			if (nparsed <= 0)
				break;
			result = true;
		} while (0);
		return result;
	}



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HttpRequest::HttpRequest() {

	}
	HttpRequest::~HttpRequest() {

	}
	void HttpRequest::PushHeaderKey(const char* inkey, const size_t& inkey_size) {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		do {
			if (!inkey || inkey_size <= 0)
				break;
			char* newkey = (char*)malloc(inkey_size);
			if (!newkey)
				break;
			memcpy(newkey, inkey, inkey_size);
			newkey = ::_strlwr(newkey);
			std::string key(newkey, inkey_size);
			free(newkey);
			auto found = m_mapHeader.find(key);
			if (found != m_mapHeader.end())
				m_mapHeader.erase(found);
			m_mapHeader.insert(std::make_pair(key, " "));
		} while (0);
	}
	void HttpRequest::PushHeaderValue(const char* invalue, const size_t& invalue_size) {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		do {
			if (!invalue || invalue_size <= 0)
				break;
			std::string value(invalue, invalue_size);
			if (m_mapHeader.empty())
				break;
			for (auto& node : m_mapHeader) {
				if (node.second == " ") {
					node.second = value;
					break;
				}
			}
		} while (0);
	}
	void HttpRequest::HeaderClear() {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		m_mapHeader.clear();
	}
	std::string HttpRequest::operator[](const std::string& key) {
		std::string result;
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		do {
			if (key.empty())
				break;
			auto found = m_mapHeader.find(key);
			if (found == m_mapHeader.end())
				break;
			result = found->second;
		} while (0);
		return result;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HttpUrlFields::HttpUrlFields() {

	}
	HttpUrlFields::~HttpUrlFields() {

	}
	void HttpUrlFields::clear() {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		m_mapUrlField.clear();
	}
	void HttpUrlFields::push(const EnHttpUrlFieldType& key, const std::string& value) {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		do {
			if (value.empty())
				break;
			auto found = m_mapUrlField.find(key);
			if (found != m_mapUrlField.end())
				m_mapUrlField.erase(key);
			m_mapUrlField.emplace(key, value);
		} while (0);
	}
	const unsigned short& HttpUrlFields::port() const {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		return m_port;
	}
	void HttpUrlFields::port(const unsigned short& port) {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		m_port = port;
	}
	bool HttpUrlFields::empty() const {
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		return m_mapUrlField.empty();
	}
	std::string HttpUrlFields::operator[](const EnHttpUrlFieldType& key) {
		std::string result;
		std::lock_guard<std::mutex> lock{ *m_Mutex };
		do {
			auto found = m_mapUrlField.find(key);
			if (found == m_mapUrlField.end())
				break;
			result = found->second;
		} while (0);
		return result;
	}
}///namespace httpparser 
#ifndef TCP_REQUEST_PARSER_HPP
#define TCP_REQUEST_PARSER_HPP

#include <tuple>
#include <string>

#ifndef _MSC_VER
#include <streambuf>
#include <iostream>
#endif

namespace vnet {

struct request;



/// Parser for incoming requests.
class request_parser
{
public:
    /// Construct ready to parse the request method.
    request_parser()
	{}

    /// Reset to initial parser state.
    void reset()
	{}

    /// Result of parse.
    enum result_type { good, bad, indeterminate };

    template <typename InputIterator>
    std::tuple<result_type, InputIterator> parse(request& req, InputIterator begin, InputIterator end)
    {
		int32_t command_code;
		int32_t num_arguments;

#if 0
		membuf sbuf(buffer, buffer + sizeof(buffer));
		std::istream in(&sbuf);

		in >> command_code;
		in >> num_arguments;

		for (int32_t i = 0; i < num_arguments; ++i)
		{
		}

        while (begin != end)
        {
            result_type result = consume(req, *begin++);
            if (result == good || result == bad)
                return std::make_tuple(result, begin);
        }
#endif
        return std::make_tuple(indeterminate, begin);
    }
};

} // namespace vnet.

#endif // TCP_REQUEST_PARSER_HPP

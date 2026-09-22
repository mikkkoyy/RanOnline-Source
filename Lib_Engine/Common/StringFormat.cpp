#include "stdafx.h"
#include "./StringFormat.h"

/*dmk14 string format*/
namespace sc {
	namespace string {
        boost::format formatter(const std::string& f_string)
        {
            using namespace boost::io;
            boost::format fmter(f_string);
            fmter.exceptions(all_error_bits ^ (too_many_args_bit | too_few_args_bit));
            return fmter;
        }
    }
}
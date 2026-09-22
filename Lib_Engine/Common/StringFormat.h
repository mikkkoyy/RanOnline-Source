/*dmk14 string format*/
#include <boost/format.hpp>
#include <boost/preprocessor.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/detail/no_exceptions_support.hpp>

/*dmk14 string format*/
namespace sc {
	namespace string
    {
        boost::format formatter( const std::string& f_string );

		#define X_GENERATE_FORMAT(z,n,x) %arg##n

		#define X_GENERATE_FORMAT_FUNCTION(z,n,d)													\
			template< BOOST_PP_ENUM_PARAMS(n,typename T)>											\
			std::string format( const std::string& strFmt, BOOST_PP_ENUM_BINARY_PARAMS(n, T, arg) )	\
			BOOST_TRY																				\
			{																						\
				using namespace boost::io;															\
				boost::format f;																	\
				f.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit )  );			\
				f.parse(strFmt);																	\
				f BOOST_PP_REPEAT(n,X_GENERATE_FORMAT,_);											\
				return f.str();																		\
			}																						\
			BOOST_CATCH(boost::io::format_error& ex){												\
				std::string Err(strFmt);															\
				Err.append(" ");                                                                    \
				Err.append(ex.what());                                                              \
				return Err;                                                                         \
			}																						\
			BOOST_CATCH_END																			\

		BOOST_PP_REPEAT_FROM_TO( 1, 50, X_GENERATE_FORMAT_FUNCTION, _)

		#undef X_GENERATE_FORMAT_FUNCTION
		#undef X_GENERATE_FORMAT
    }
}

/*dmk14 string format*/
#define stringformat sc::string::format
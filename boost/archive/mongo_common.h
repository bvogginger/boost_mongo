#pragma once

// Copyright (c) 2012, Sebastian Jeltsch (sjeltsch@kip.uni-heidelberg.de)
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <mongo/client/dbclient.h>

#define FUSION_MAX_VECTOR_SIZE 20
#define FUSION_MAX_MAP_SIZE 20

#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/value_at_key.hpp>

#include <boost/archive/basic_archive.hpp>

namespace boost {
namespace archive {
namespace detail {

template<typename T>
class cond_deleter
{
private:
	bool cond;
public:
	cond_deleter(bool del = true) : cond(del) {}

	void operator() (T* b)
	{
		if (cond)
			delete b;
	}
};


template<typename T>
class has_compare
{
private:
	// better match if SFINAE doesn't fail
	template<typename U>
	static constexpr bool fun(decltype(&U::operator==)*) { return true; }

	template<typename U>
	static constexpr bool fun(...) { return false; }
public:
	enum : bool { value = fun<T>(nullptr) };
};


template<typename T>
struct is_compressable
{
	enum : bool { value =
		std::is_default_constructible<T>::value &&
		(detail::has_compare<T>::value || std::is_arithmetic<T>::value)
		   };
};

} // detail


// BSONObj supports the following native C++ types:
// long long, bool, int, double, std::string
// This means all other types need to be mapped accordingly
typedef fusion::map<
		fusion::pair<char, int>,
		fusion::pair<signed char, int>,
		fusion::pair<unsigned char, int>,
		fusion::pair<wchar_t, int>,
		fusion::pair<char16_t, int>,
		fusion::pair<char32_t, long>,
		fusion::pair<short, int>,
		fusion::pair<unsigned short, int>,
		fusion::pair<unsigned int, long long>,
		fusion::pair<long, long long>,
		fusion::pair<unsigned long, long long>,
		fusion::pair<unsigned long long, long long>, // problematic
		fusion::pair<float, double>,
		fusion::pair<long double, double>            // problematic
	> bson_type_mapping;


typedef fusion::map<
		fusion::pair<archive::class_id_type, int16_t>,
		fusion::pair<archive::class_id_optional_type, int16_t>, // strong reference to class_id_type
		fusion::pair<archive::class_id_reference_type, int16_t>, // strong reference to class_id_type
		fusion::pair<archive::object_id_type, uint32_t>,
		fusion::pair<archive::object_reference_type, uint16_t>, // strong typedef to object_id_type
		fusion::pair<archive::version_type, uint32_t>,
		fusion::pair<archive::tracking_type, bool>
		//fusion::pair<archive::class_name_type, char const*>
	> meta_type_mapping;

typedef fusion::map<
		fusion::pair<archive::class_id_type, char const*>,
		fusion::pair<archive::class_id_optional_type, char const*>,
		fusion::pair<archive::class_id_reference_type, char const*>,
		fusion::pair<archive::object_id_type, char const*>,
		fusion::pair<archive::object_reference_type, char const*>,
		fusion::pair<archive::version_type, char const*>,
		fusion::pair<archive::tracking_type, char const*>
		//fusion::pair<archive::class_name_type, char const*>
	> meta_type_names_t;

meta_type_names_t const meta_type_names(
	fusion::make_pair<archive::class_id_type>("_class_id"),
	fusion::make_pair<archive::class_id_optional_type>("_class_id_optional"),
	fusion::make_pair<archive::class_id_reference_type>("_class_id_reference"),
	fusion::make_pair<archive::object_id_type>("_object_id"),
	fusion::make_pair<archive::object_reference_type>("_object_reference"),
	fusion::make_pair<archive::version_type>("_version"),
	fusion::make_pair<archive::tracking_type>("_tracking")
	//fusion::make_pair<archive::class_name_type>("_class_name")
);

} // archive
} // boost
#include <iostream>
#include <gtest/gtest.h>

#include <boost/foreach.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/array.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "boost/archive/mongo_oarchive.hpp"
#include "boost/archive/mongo_iarchive.hpp"

using namespace boost::archive;
using boost::serialization::make_nvp;

typedef ::testing::Types<
		bool,
		char,
		signed char,
		unsigned char,
		wchar_t,
		//char16_t,
		//char32_t,
		short,
		unsigned short,
		int,
		unsigned int,
		long,
		unsigned long,
		long long,
		unsigned long long,
		float,
		long double,
		double
	> types;

template<typename T>
struct MongoBuiltin :
	public ::testing::Test
{
	typedef T type;
};

TYPED_TEST_CASE(MongoBuiltin, types);

TYPED_TEST(MongoBuiltin, BaseTypes)
{
	mongo::BSONObjBuilder builder;
	mongo_oarchive out(builder);

	typename TestFixture::type a, b;
	a = 42;
	out << make_nvp("value", a);

	mongo::BSONObj obj = builder.obj();
	ASSERT_TRUE(!obj.toString().empty());

	mongo_iarchive in(obj);
	in >> make_nvp("value", b);

	ASSERT_EQ(a, b);
}

struct A
{
	int a;
	unsigned int b;
	enum Name { x, y, z };
	Name n;

	//char const ch [6] = "hello";
	std::string str;

	A() : str("me string") {}

	bool operator== (A const& other) const
	{
		return a == other.a &&
			b == other.b &&
			n == other.n &&
			//ch == other.ch &&
			str == other.str;
	}

private:
	friend class boost::serialization::access;
	template<typename Archive>
	void serialize(Archive& ar, unsigned int const)
	{
		using namespace boost::serialization;
		ar & BOOST_SERIALIZATION_NVP(a)
		   & BOOST_SERIALIZATION_NVP(b)
		   & make_nvp("enum", n)
		   //& make_nvp("ch", ch)
		   & make_nvp("str", str);
	}
};

TEST(MongoArchive, CustomType)
{
	const char name [] = { "myType" };
	A a, b;
	a.n = static_cast<A::Name>(42);
	a.str = "hihihi";

	mongo::BSONObjBuilder builder;
	mongo_oarchive out(builder);

	out << make_nvp(name, a);

	mongo::BSONObj o = builder.obj();
	int e = o.getField(name).embeddedObject().getField("enum").Int();
	//ASSERT_EQ("hello",  o.getField(name).embeddedObject().getField("ch").String());
	ASSERT_EQ("hihihi",  o.getField(name).embeddedObject().getField("str").String());

	ASSERT_EQ((int)a.n, e);

	mongo_iarchive in(o);
	in >> make_nvp(name, b);

	ASSERT_EQ(a, b);
}

TEST(MongoArchive, AllMembersRegressionTest)
{
	const char name [] = { "myArray" };
	mongo::BSONObjBuilder builder;
	mongo_oarchive mongo(builder);

	boost::array<int, 42> a;
	mongo << make_nvp(name, a);

	mongo::BSONObj o = builder.obj();
	mongo::BSONElement elem = o.getField(name).embeddedObject().getField("elems");
	ASSERT_EQ(static_cast<int>(a.size()+1), elem.embeddedObject().nFields());
}

TEST(MongoArchive, Array)
{
	const char name [] = { "myArray" };
	typedef boost::array<int, 1024> type;
	type a, b;

	mongo::BSONObjBuilder builder;
	mongo_oarchive out(builder);

	size_t cnt = 0;
	BOOST_FOREACH(int& val, a)
	{
		val = cnt++;
	}

	out << make_nvp(name, a);

	mongo::BSONObj o = builder.obj();

	mongo_iarchive in(o);
	in >> make_nvp(name, b);

	ASSERT_EQ(a, b);
}

TEST(MongoArchive, SparseArray)
{
	const char name [] = { "myArray" };
	typedef boost::array<int, 1024> type;
	type a, b;

	mongo::BSONObjBuilder builder;
	mongo_oarchive out(builder, mongo_oarchive::sparse_array);

	// zero all entries
	BOOST_FOREACH(int& val, a)
	{
		val = 0;
	}

	// set some to individual values
	int start=10, range=10;
	for(int ii = start; ii<start+range; ++ii)
		a[ii] = ii;

	out << make_nvp(name, a);

	mongo::BSONObj o = builder.obj();

	mongo::BSONElement elem = o.getField(name).embeddedObject().getField("elems");
	ASSERT_EQ(range+1, elem.embeddedObject().nFields());

	mongo_iarchive in(o, mongo_iarchive::sparse_array);
	in >> make_nvp(name, b);

	ASSERT_EQ(a, b);
}

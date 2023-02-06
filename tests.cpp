#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <random>
#include <time.h>
#define SMART_POINTER_NTS_PRINT_LOG
#include "smart_pointer_nts.h"
using namespace smart_pointer_nts;


struct test
{
	int x = 1;
	int y = -1;
	test(int x = 0, int y = 0) :x(x), y(y) {}
};


void TestSharedPointer() 
{
	std::cout << "TestSharedPointer.." << std::endl;

	srand(time(NULL));

	// initial value
	test* resource0 = new test;
	shared_ptr<test> sp1(resource0);
	assert(sp1.use_count() == 1);

	// copy constractor
	auto sp2 = sp1;
	assert(sp1.use_count() == 2);

	// move constructor
	auto sp3 = std::move(sp1);
	assert(sp3.use_count() == 2);
	assert(sp1.use_count() == 0);

	// copy assignment
	shared_ptr<test> spB1(new test);
	sp2 = spB1;
	assert(sp3.use_count() == 1);
	assert(sp2.use_count() == 2);
	assert(spB1.use_count() == 2);
	assert(sp2 == sp3 == false);
	assert(sp2 == spB1 == true);
	assert(sp2 == nullptr == false);

	// initial value (null)
	shared_ptr<int> spNul;
	assert(spNul.use_count() == 0);

	{
		// reset as null
		shared_ptr<int> spRst(new int(rand()));
		spRst.reset();
		assert(spRst.use_count() == 0);

		// reset with an other.
		spRst.reset(new int(rand()));
		assert(spRst.use_count() == 1);
		spRst.reset(new int(rand()));
		assert(spRst.use_count() == 1);
	}

}

void TestWeakPointer()
{
	std::cout << "TestWeakPointer.." << std::endl;

	test* resource0 = new test;
	shared_ptr<test> sp0(resource0);

	// weak ptr
	weak_ptr<test> wp1 = sp0;
	assert(wp1.expired() == false);
	auto wp2 = wp1;
	auto wp3 = std::move(wp2);
	wp2 = wp3;

	// shared from weak
	auto wsp = wp1.lock();
	assert(wsp.get() == resource0);
	assert(wsp->x == test().x);

	// reset 
	{
		int cnt = 0;
		auto deleter = [&cnt](void* obj) { delete (test*)obj; cnt = 1; };
		auto raw3 = new test();
		unique_ptr<test, std::function<void(void*)>> uq3(raw3, deleter);

		assert(cnt == 0);
		uq3.reset(new test());
		assert(cnt == 1);
	}
}

void TestUniquePointer()
{
	std::cout << "TestUniquePointer.." << std::endl;

	// unique ptr
	auto raw1 = new test();
	unique_ptr<test> uq1(raw1);
	unique_ptr<test> uq2;
	uq2 = std::move(uq1);
	assert((bool)uq1 == false);
	assert((bool)uq2 == true);
	assert(!(uq1 == uq2));
	assert(uq1 == nullptr);
	auto raw2 = new test(-1, -2);
	uq2.reset(raw2);
	assert(uq2.get() == raw2);
	assert(uq2->y == -2);
}

void TestHashValue() 
{
	std::cout << "TestHashValue.." << std::endl;

	srand(time(NULL));
	auto val = rand();

	assert(
		std::hash<shared_ptr<int>>()(shared_ptr<int>(new int(val)))
		!= std::hash<shared_ptr<int>>()(shared_ptr<int>(new int(val)))
	);
}

void TestEqualValue() 
{
	std::cout << "TestEqualValue.." << std::endl;

	srand(time(NULL));
	auto val = rand();

	assert(
		!std::equal_to<shared_ptr<int>>()(
			shared_ptr<int>(new int(val)),
			shared_ptr<int>(new int(val)))
	);
}

void TestEtcetra()
{
	std::cout << "TestEtcetra.." << std::endl;

	// manage array resource
	weak_ptr<test[]> wary1;
	{
		auto ary = new test[3]{ {0,1}, {2,3}, {4,5} };
		shared_ptr<test[]> ary1(ary);
		wary1 = ary1;
		auto ary2 = ary1;
		assert(ary1[1].x == 2);
	}

	// try access an disposed resource.
	assert(wary1.expired() == true);
	assert((bool)wary1.lock() == false);

	// using smart ptr as key on unordered_set
	std::unordered_set<shared_ptr<test>> set0;
	set0.emplace(new test(1, 2));
	set0.insert(shared_ptr<test>(new test(3, 4)));
	shared_ptr<test> setTest(new test(5, 6));
	set0.insert(setTest);
	assert(set0.size() == 3);
	set0.insert(setTest);
	assert(set0.size() == 3);
}

int main()
{
	TestSharedPointer();
	TestWeakPointer();
	TestUniquePointer();
	TestHashValue();
	TestEqualValue();
	TestEtcetra();

	return 0;
}

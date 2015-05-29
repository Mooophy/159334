#include "stdafx.h"
#include "CppUnitTest.h"
#include "../lib/as3.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{		
	TEST_CLASS(Rsa)
	{
	public:
		
		TEST_METHOD(key_ctor)
		{
            auto key = as3::rsa::Key{ 1, 2, 3 };
            Assert::AreEqual(1, key.n);
            Assert::AreEqual(2, key.e);
            Assert::AreEqual(3, key.d);
		}

	};
}
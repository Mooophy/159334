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
            auto rkey = as3::rsa::RsaKey{ 1, 2, 3 };
            Assert::AreEqual((char)1, rkey.n);
            Assert::AreEqual((char)2, rkey.e);
            Assert::AreEqual((char)3, rkey.d);
        }

        TEST_METHOD(RsaKeyList)
        {
            using RK = as3::rsa::RsaKey;
            auto keys = { RK{ 143, 7, 103 }, RK{ 187, 27, 83 }, RK{ 209, 17, 53 } };
            auto key_list = as3::rsa::RsaKeyList(keys.begin(), keys.end());

            Assert::AreEqual(3u, key_list.data().size());
        }

    };
}
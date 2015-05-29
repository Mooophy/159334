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
            auto tkey = as3::rsa::TriKey{ 1, 2, 3 };
            Assert::AreEqual(1, tkey.n);
            Assert::AreEqual(2, tkey.e);
            Assert::AreEqual(3, tkey.d);
        }

        TEST_METHOD(repeat_square_set1)
        {
            auto key = as3::rsa::TriKey{ 143, 7, 103 };
            for (int i = 0; i != 127; ++i)
            {
                auto en = as3::rsa::repeat_square(i, key.e, key.n);
                auto de = as3::rsa::repeat_square(en, key.d, key.n);
                Assert::AreEqual(i, de);
            }
        }

        TEST_METHOD(repeat_square_set2)
        {
            auto key = as3::rsa::TriKey{ 187, 27, 83 };
            for (int i = 0; i != 127; ++i)
            {
                auto en = as3::rsa::repeat_square(i, key.e, key.n);
                auto de = as3::rsa::repeat_square(en, key.d, key.n);
                Assert::AreEqual(i, de);
            }
        }

        TEST_METHOD(repeat_square_set3)
        {
            auto key = as3::rsa::TriKey{ 209, 17, 53 };
            for (int i = 0; i != 127; ++i)
            {
                auto en = as3::rsa::repeat_square(i, key.e, key.n);
                auto de = as3::rsa::repeat_square(en, key.d, key.n);
                Assert::AreEqual(i, de);
            }
        }


        TEST_METHOD(RsaKeyList)
        {
            using TK = as3::rsa::TriKey;
            auto keys = { TK{ 143, 7, 103 }, TK{ 187, 27, 83 }, TK{ 209, 17, 53 } };
            auto key_list = as3::rsa::RsaKeyList(keys.begin(), keys.end());

            Assert::AreEqual(3u, key_list.data().size());
        }

    };
}
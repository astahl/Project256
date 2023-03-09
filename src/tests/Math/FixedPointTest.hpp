//
//  FixedPointTest.hpp
//  Project256
//
//  Created by Andreas Stahl on 18.12.22.
//
#pragma once

#include "Test.hpp"
#include "../../game/Math/FixedPoint.hpp"

namespace FixedPointTest {

void somethingWorks(Test& t)
{
    FixedPointReal<1, int> a{2.5};
    FixedPointReal<2, int> b{a};
    FixedPointReal<3, int> c = a;
    t.expect(static_cast<int>(a), 2);
    t.expect(b.data, 0b1010);
    t.expect(c.data, 0b10100);

    t.expect(a.toFloat(), 2.5);

    FixedPointReal<0, int> d = a;
    t.expect(d.data, 0b10);
    int implicit = d;
    t.expect(d.toInt(), implicit);

    FixedPointReal<-1, int> e = a;
    t.expect(e.data, 0b1);

    FixedPointReal<8, uint8_t> f{0.5};
    t.expect(f.data, 128);

    FixedPointReal<-4, int> g{16.5};
    t.expect(g.toFloat(), 16.0);

}

void floatConversion(Test& t)
{
    FixedPointReal<28, int> pi{3.141};
    t.expect(pi.toFloat(), Matchers::IsNear<double, 6>{3.141});

    FixedPointReal<20, int> a{-5.145};
    t.expect(a.toFloat(), Matchers::IsNear<double, 6>{-5.145});
}

void negativeHandling(Test& t)
{
    FixedPointReal<8, int> a{-1};
    t.expect(a.toInt(), -1);
    t.expect(a.toFloat(), Matchers::IsNear<double, 6>{-1.0});
}

void multiplication(Test& t)
{
    FixedPointReal<8, int> a{5};
    auto b = a * 5;
    t.expect(b.toInt(), 25);

    auto c = b * -0.2;
    t.expect(c.toInt(), -5);

    auto d = c * a;
    static_assert(d.precision == 16);
    t.expect(d.toInt(), -25);

    FixedPointReal<8,int> e = c * a;
    t.expect(e.toInt(), -25);


    FixedPointReal<8, int> f{10};
    FixedPointReal<8, int> g{-0.5};
    auto h = f * g;
    t.expect(h.toInt(), -5);

}

void addition(Test& t)
{
    FixedPointReal<8, int> a{5};
    auto b = a + 5;
    static_assert(b.precision == 8);
    t.expect(b.toInt(), 10);

    auto c = b - 10.5;
    static_assert(c.precision == 8);
    t.expect(c.toInt(), -1);
    t.expect(c.toFloat(), -0.5);

    auto d = a - c;
    static_assert(d.precision == 8);
    t.expect(d.toInt(), 5);
    t.expect(d.toFloat(), 5.5);

    auto e = a + c;
    static_assert(e.precision == 8);
    t.expect(e.toInt(), 4);
    t.expect(e.toFloat(), 4.5);
}

void addAll(Test& t)
{
    t.add(FixedPointTest::somethingWorks);
    t.add(FixedPointTest::floatConversion);
    t.add(FixedPointTest::negativeHandling);
    t.add(multiplication);
    t.add(addition);
}

}

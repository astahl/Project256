//
//  TrigonometryTest.hpp
//  Project256
//
//  Created by Andreas Stahl on 12.12.22.
//

#pragma once
#include "../Test.hpp"

#include "../../game/Math/Trigonometry.hpp"
#include <cmath>

void test_myCos(Test& test) {
    using IsNear = Matchers::IsNear<double, 2>;

    static_assert(std::predicate<IsNear, double>);

    for (double x = 0.0; x < 10.0; x += 0.01) {
        double my = myCos(x);
        double std = std::cos(x);
        auto pred = IsNear{ std };
        test.expect(my, pred);
        my = my; std = std;
    }

    for (double x = 0.0; x > -10.0; x -= 0.01) {
        double my = myCos(x);
        double std = std::cos(x);
        test.expect(my, IsNear{std});
        my = my; std = std;
    }

    for (double x = -10.0; x < 10.0; x += 0.01) {
        double my = mySin(x);
        double std = std::sin(x);
        test.expect(my, IsNear{std});
        my = my; std = std;
    }
}

//
//  TestsMain.cpp
//  Project256
//
//  Created by Andreas Stahl on 12.12.22.
//

#include "Test.hpp"

#include "Math/TrigonometryTest.hpp"

int main() {
    Test t{};
    t.add(test_myCos);
    return t.run();
}

//
//  LinAlg.hpp
//  Project256
//
//  Created by Andreas Stahl on 13.03.23.
//


#pragma once
#include "../Test.hpp"
#include "../../game/Math/LinAlg.hpp"
void testMatrix(Test& t)
{
    Matrix<int,1,1> m{4};
    t.expect(static_cast<int>(m), 4);

    Matrix<int, 1, 2> v1{2, 2};
    Matrix<int, 2, 1> v2{1, 1};
    
    auto r = v1 * v2;
    t.expect(r, 4);

    auto added = v2 + v2;
    t.expect(added[0], 2);
    t.expect(added[1], 2);

    auto x = Matrix<int,1,4>{1,2,0,2};
    auto y = x.transposed();
    t.expect(x * y, 9);

    auto scaledLeft = 3 * x;
    t.expect(scaledLeft(0,3), 6);
    auto divRight = scaledLeft / 2;
    t.expect(divRight(0,1), 3);

    auto divLeft = 1.0 / x;
    t.expect(divLeft(0,3), 0.5);

    auto a = ColumnVector<int,4>{1,2,3,4};
    auto b = RowVector<int,4>{4,3,2,1};
    
}

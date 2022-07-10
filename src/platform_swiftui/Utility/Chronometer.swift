//
//  Chronometer.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation


class Chronometer {
    var lastTimeIndex = 0
    var timevalues = UnsafeMutableBufferPointer<timeval>.allocate(capacity: 2)

    func elapsed() -> (microseconds: Int64, seconds: Double) {
        let nextTimeIndex = lastTimeIndex == 0 ? 1 : 0;
        gettimeofday(&timevalues[nextTimeIndex], nil)
        let t0 = timevalues[lastTimeIndex]
        let t1 = timevalues[nextTimeIndex]
        self.lastTimeIndex = nextTimeIndex
        let ms = Int64(t1.tv_sec - t0.tv_sec) * 1_000_000 + Int64(t1.tv_usec - t0.tv_usec)
        return (microseconds: ms, seconds: Double(ms) / 1_000_000);
    }

    init() {
        gettimeofday(&timevalues[lastTimeIndex], nil)
    }
}

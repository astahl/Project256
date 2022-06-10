//
//  Timings.swift
//  Project256
//
//  Created by Andreas Stahl on 10.06.22.
//

import Foundation

enum TimingInterval: CaseIterable {
    case FrameToFrame
    case TickSetup, TickDo, TickPost
    case BufferCopy
    case DrawWaitAndSetup, DrawEncoding, DrawPresent
}

class Timings {
    var timings: [TimingInterval: [Int64]] = [:]
    public static let global: Timings? = Timings(capacity: 100)

    init(capacity: Int) {
        timings.reserveCapacity(TimingInterval.allCases.count)
        for interval in TimingInterval.allCases {
            var array = Array<Int64>()
            array.reserveCapacity(capacity)
            timings[interval] = array
        }
    }

    func addTiming(for interval: TimingInterval, µs: Int64)
    {
        timings[interval]?.append(µs)
    }

    func clear() {
        for var (_, list) in timings {
            list.removeAll()
        }
    }

    var description: String {
        get {
            timings.compactMap() {
                (key, list) in
                if list.isEmpty { return nil }
                let sum = list.reduce(into: Int64(0)) { $0 += $1 }
                return "\(key): \(sum / Int64(list.count))"
            }.joined(separator: ", ")
        }
    }

}

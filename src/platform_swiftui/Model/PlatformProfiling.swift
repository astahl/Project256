//
//  PlatformProfiling.swift
//  Project256
//
//  Created by Andreas Stahl on 24.07.22.
//

import Foundation


class PlatformProfiling {
    static let instance = PlatformProfiling()
    static let lock = NSLock()
    public static func withInstance(closure:(_:PlatformProfiling) -> Void) {
        lock.lock()
        defer{ lock.unlock() }
        closure(instance)
    }

    public var timingData: ProfilingTime

    private init() {
        timingData = ProfilingTime()
        timingData.getPlatformTimeMicroseconds = timestamp
    }
}

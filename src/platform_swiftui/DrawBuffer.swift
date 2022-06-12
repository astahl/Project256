//
//  DrawBuffer.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation

class DrawBuffer {
    let width: Int
    let height: Int
    let aspectRatio: Float
    let data: UnsafeMutableBufferPointer<UInt32>

    init() {
        self.aspectRatio = Float(DrawAspectH) / Float(DrawAspectV)
        self.width = Int(DrawBufferWidth)
        self.height = Int(DrawBufferHeight)
        data = UnsafeMutableBufferPointer<UInt32>.allocate(capacity: width * height)
    }
}

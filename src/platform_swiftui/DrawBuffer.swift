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

    convenience init(width: Int, height: Int) {
        self.init(width: width, height: height, aspectHorizontal: width, aspectVertical: height)
    }

    init(width: Int, height: Int, aspectHorizontal: Int, aspectVertical: Int) {
        self.aspectRatio = Float(aspectHorizontal) / Float(aspectVertical)
        self.width = width
        self.height = height
        data = UnsafeMutableBufferPointer<UInt32>.allocate(capacity: width * height)
    }
}

//
//  DrawBuffer.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI


extension CGColor {
    func toUInt32ARGB(intent: CGColorRenderingIntent = .defaultIntent) -> UInt32? {
        var maybeColor: CGColor? = self

        if ![.rgb, .monochrome].contains(maybeColor!.colorSpace?.model) {
            if let colorSpace = CGColorSpace(name:CGColorSpace.linearSRGB) {
                maybeColor = maybeColor?.converted(to: colorSpace, intent: intent, options: .none)
            }
        }

        guard let color = maybeColor else {
            return nil;
        }

        var a, r, g, b: UInt32
        switch color.numberOfComponents {
        case 1:
            a = 255; r = UInt32(color.components![0] * 255); g = r; b = r
        case 2:
            a = UInt32(color.components![1] * 255); r = UInt32(color.components![0] * 255); g = r; b = r
        case 3:
            a = 255
            r = UInt32(color.components![0] * 255)
            g = UInt32(color.components![1] * 255)
            b = UInt32(color.components![2] * 255)
        case 4:
            a = UInt32(color.components![3] * 255)
            r = UInt32(color.components![0] * 255)
            g = UInt32(color.components![1] * 255)
            b = UInt32(color.components![2] * 255)
        default:
            a = 255; r = 255; g = 255; b = 255
        }

        return a << 24 | r << 16 | g << 8 | b
    }
}


class DrawBuffer {
    enum TestPattern {
        case SolidColor (CGColor)
        case Checkerboard (CGColor, CGColor, size: UInt32 = 8)
    }
    
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

    convenience init(withTestPattern testPattern: TestPattern) {
        self.init()

        switch testPattern {
        case .SolidColor(let color):
            if let colorInt = color.toUInt32ARGB() {
                self.data.assign(repeating: colorInt)
            }
        case .Checkerboard(let color1, let color2, let size):
            if let colorInt1 = color1.toUInt32ARGB(),
               let colorInt2 = color2.toUInt32ARGB() {
                for y in 0..<DrawBufferHeight {
                    for x in 0..<DrawBufferWidth {
                        let a = (y / size % 2 == 1)
                        let b = (x / size % 2 == 1)

                        self.data[Int(x + y * DrawBufferWidth)] = a && !b || !a && b ? colorInt1 : colorInt2
                    }
                }
            }
        }
    }
}

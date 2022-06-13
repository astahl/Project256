//
//  DrawBufferView.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import SwiftUI

typealias DrawBufferSource = (_ oldDrawBuffer: DrawBuffer?) -> DrawBuffer?

struct DrawBufferView: View {
    var drawBuffer: DrawBuffer?
    var drawBufferSource: DrawBufferSource?

    let metalView: MetalView

    init(drawBuffer: DrawBuffer)
    {
        self.drawBuffer = drawBuffer
        metalView = MetalView(drawBuffer: drawBuffer)
    }

    init(drawBufferSource: @escaping DrawBufferSource)
    {
        self.drawBufferSource = drawBufferSource
        metalView = MetalView(drawBuffer: nil)
    }

    func pixelPosition(_ locationInView: CGPoint) -> CGPoint?
    {
        return metalView.pixelPosition?(locationInView)
    }

    var body: some View {
        metalView
            .beforeDraw {
                drawBuffer in
                if let source = drawBufferSource {
                    return source(drawBuffer)
                }
                return drawBuffer
            }
    }
}

struct DrawBufferView_Previews: PreviewProvider {
    static var previews: some View {
        DrawBufferView(
            drawBuffer: .init(withTestPattern: .Checkerboard(.black, .init(red: 1.0, green: 0, blue: 0, alpha: 1), size: 10)))
    }
}

//
//  DrawBufferView.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import SwiftUI

typealias DrawBufferSource = (_ oldDrawBuffer: DrawBuffer?) -> DrawBuffer?

var pixelPositionConv: PointConversion?

struct DrawBufferView: View {
    var drawBuffer: DrawBuffer?
    var drawBufferSource: DrawBufferSource?
    var metalView: MetalView

    init(drawBuffer: DrawBuffer)
    {
        self.drawBuffer = drawBuffer
        metalView = MetalView(drawBuffer: drawBuffer, beforeDraw: nil)
        metalView.updateHandler = self.onMtkViewUpdate(view:)
    }

    init(drawBufferSource: @escaping DrawBufferSource)
    {
        self.drawBufferSource = drawBufferSource
        metalView = MetalView(drawBuffer: nil, beforeDraw: {
            drawBuffer in
            return drawBufferSource(drawBuffer)
        })
        metalView.updateHandler = self.onMtkViewUpdate(view:)
    }

    func pixelPosition(_ locationInView: CGPoint?, flipY: Bool) -> CGPoint?
    {
        if let location = locationInView {
            return pixelPositionConv?(location, flipY)
        }
        return nil
    }

    func onMtkViewUpdate(view: MyMTKView) {
        pixelPositionConv = view.positionOnBuffer(locationInView:flipY:)
    }

    var body: some View {
        metalView
    }
}

struct DrawBufferView_Previews: PreviewProvider {
    static var previews: some View {
        DrawBufferView(
            drawBuffer: .init(withTestPattern: .Checkerboard(.init(gray: 0.0, alpha: 1.0), .init(red: 1.0, green: 0, blue: 0, alpha: 1), size: 10)))
        .background(.gray)
    }
}

//
//  PlatformCallbacks.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation
import CoreImage


func loadDataDEBUG(filenamePtr: UnsafePointer<CChar>?, destination: UnsafeMutablePointer<UInt8>?, bufferSize: Int64) -> Int64 {
    let filename = String(cString: filenamePtr!)
    let url = Bundle.main.url(forResource: filename, withExtension: nil)
    let data = try? Data(contentsOf: url!)
    let count = min(bufferSize, Int64(data!.count))
    data!.copyBytes(to: destination!, count: Int(count))
    return count
}

func loadImageDEBUG(filenamePtr: UnsafePointer<CChar>?, destination: UnsafeMutablePointer<UInt32>?, width: Int32, height: Int32) -> Bool {
    let filename = String(cString: filenamePtr!)
    let url = Bundle.main.url(forResource: filename, withExtension: nil)
    let image = CIImage.init(contentsOf: url!)!
    let cgImage = image.cgImage ?? {
        let context = CIContext()
        return context.createCGImage(image, from: image.extent)!
    }()
    let targetColorSpace = CGColorSpace.init(name: CGColorSpace.sRGB)!;
    guard let context = CGContext.init(data: destination, width: Int(width), height: Int(height), bitsPerComponent: 8, bytesPerRow: 4 * Int(width), space: targetColorSpace, bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue) else {
        return false
    }

    context.draw(cgImage, in: CGRect(x: 0, y: 0, width: Int(width), height: Int(height)))
    // swizzle BGRA to ARGB with byte swap
    let buffer = UnsafeMutableBufferPointer(start: destination, count: Int(width * height))
    for i in 0..<buffer.count {
        buffer[i] = buffer[i].byteSwapped
    }
    return true
}

func timestamp() -> Int64 {
    var time = timeval()
    gettimeofday(&time, nil)
    return Int64(time.tv_sec) * 1_000_000 + Int64(time.tv_usec)
}

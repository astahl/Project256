//
//  TupleAccess.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation


func withUnsafeElementPointer<T, Result>(firstElement: inout T, offset: Int, body: @escaping (UnsafePointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafePointer(to: &firstElement) {
            beginPtr in
            let ptr = offset == 0 ? beginPtr : beginPtr.advanced(by: offset)
            do {
                return try body(ptr)
            } catch {
                throw error
            }
        }
    } catch {
        throw error
    }
}

func withUnsafeMutableElementPointer<T, Result>(firstElement: inout T, offset: Int, body: @escaping  (UnsafeMutablePointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafeMutablePointer(to: &firstElement) {
            beginPtr in
            let ptr = offset == 0 ? beginPtr : beginPtr.advanced(by: offset)
            do {
                return try body(ptr)
            } catch {
                throw error
            }
        }
    } catch {
        throw error
    }
}

func withUnsafeMutableBuffer<T, Result>(start: inout T, end: inout T, body: @escaping  (UnsafeMutableBufferPointer<T>) throws -> Result) rethrows -> Result {
    do {
        return try withUnsafeMutablePointer(to: &start) {
            startPtr in
            return try withUnsafeMutablePointer(to: &end) {
                endPtr in
                let count = startPtr.distance(to: endPtr) + 1
                do {
                    return try body(UnsafeMutableBufferPointer(start: startPtr, count: count))
                } catch {
                    throw error
                }
            }
        }
    } catch {
        throw error
    }
}

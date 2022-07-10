//
//  MouseModifiers.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import Foundation
import SwiftUI

#if os(macOS)
extension NSPoint {
    var cgPoint: CGPoint { get { .init(x: self.x, y: self.y) }}
}

class KbMFirstResponderView : NSView {

    typealias KeyboardHandler = (_ event: KeyboardEvent) -> Void
    typealias MouseClickHandler = (_ event: MouseClickEvent) -> Void
    typealias MouseMoveHandler = (_ event: MouseMoveEvent) -> Void

    var keyboardHandler: KeyboardHandler?
    var moveEventHandler: MouseMoveHandler?
    var clickEventHandler: MouseClickHandler?

    override func viewDidMoveToWindow() {
        self.window?.acceptsMouseMovedEvents = true
    }

    override func resetCursorRects() {
        self.addCursorRect(self.bounds, cursor: .crosshair)
    }

    override var acceptsFirstResponder: Bool { get { true } }

    override func becomeFirstResponder() -> Bool {
        return super.becomeFirstResponder()
    }

    override func resignFirstResponder() -> Bool {
        return super.resignFirstResponder()
    }

    override func keyDown(with event: NSEvent) {
        self.keyboardHandler?(.Down(keyCode: event.keyCode, characters: event.characters, modifierFlags: event.modifierFlags))
    }

    override func keyUp(with event: NSEvent) {
        self.keyboardHandler?(.Up(keyCode: event.keyCode, characters: event.characters, modifierFlags: event.modifierFlags))
    }

    override func mouseMoved(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.moveEventHandler?(
            .Move(locationInView: locationInView, relative: CGPoint(x: event.deltaX, y: event.deltaY)))
    }

    override func scrollWheel(with event: NSEvent) {
        self.moveEventHandler?(
            .Scroll(scroll: CGPoint(x: event.scrollingDeltaX, y: event.scrollingDeltaY)))
    }

    override func mouseDragged(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.moveEventHandler?(
            .Drag(locationInView: locationInView, relative: CGPoint(x: event.deltaX, y: event.deltaY), button: .Left))
    }

    override func rightMouseDragged(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.moveEventHandler?(
            .Drag(locationInView: locationInView, relative: CGPoint(x: event.deltaX, y: event.deltaY), button: .Right))
    }

    override func otherMouseDragged(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.moveEventHandler?(
            .Drag(locationInView: locationInView, relative: CGPoint(x: event.deltaX, y: event.deltaY), button: .Other))
    }

    override func mouseDown(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Down(button: .Left, locationInView: locationInView))
    }

    override func mouseUp(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Up(button: .Left, locationInView: locationInView))
    }

    override func rightMouseDown(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Down(button: .Right, locationInView: locationInView))
    }

    override func rightMouseUp(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Up(button: .Right, locationInView: locationInView))
    }

    override func otherMouseDown(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Down(button: .Other, locationInView: locationInView))
    }

    override func otherMouseUp(with event: NSEvent) {
        let locationInView = self.convert(event.locationInWindow, from: nil).cgPoint
        self.clickEventHandler?(.Up(button: .Other, locationInView: locationInView))
    }
}

struct KbMFirstResponderViewRepresentable : NSViewRepresentable {
    var keyboard: KbMFirstResponderView.KeyboardHandler?
    var move: KbMFirstResponderView.MouseMoveHandler?
    var click: KbMFirstResponderView.MouseClickHandler?

    func makeNSView(context: Context) -> KbMFirstResponderView {
        let view = KbMFirstResponderView()
        view.keyboardHandler = keyboard
        view.moveEventHandler = move
        view.clickEventHandler = click
        return view
    }

    func updateNSView(_ nsView: KbMFirstResponderView, context: Context) {
        nsView.keyboardHandler = keyboard
        nsView.moveEventHandler = move
        nsView.clickEventHandler = click
    }

    typealias NSViewType = KbMFirstResponderView
}


struct KeyboardAndMouseModifier : ViewModifier {
    var representable: KbMFirstResponderViewRepresentable

    init(keyboard: KbMFirstResponderView.KeyboardHandler?,
         move: KbMFirstResponderView.MouseMoveHandler?,
         click: KbMFirstResponderView.MouseClickHandler?) {
        self.representable = KbMFirstResponderViewRepresentable(keyboard: keyboard, move: move, click: click)
    }

    func body(content: Content) -> some View {
        content.overlay(content: { self.representable })
    }
}


extension View {
    func keyboardAndMouse(keyboard: KbMFirstResponderView.KeyboardHandler?, move: KbMFirstResponderView.MouseMoveHandler?, click: KbMFirstResponderView.MouseClickHandler?)-> some View {
        return modifier(KeyboardAndMouseModifier(keyboard: keyboard, move: move, click: click))
    }
}

#else

#endif

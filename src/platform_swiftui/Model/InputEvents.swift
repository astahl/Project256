//
//  InputEvents.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation
import AppKit

enum KeyboardEvent {
    case Down (keyCode: CGKeyCode, characters: String?, modifierFlags: NSEvent.ModifierFlags)
    case Up (keyCode: CGKeyCode, characters: String?, modifierFlags: NSEvent.ModifierFlags)
}

enum MouseButton : Int {
    case Left, Right, Other
}

enum MouseClickEvent {
    case Down(button: MouseButton, locationInView: CGPoint)
    case Up(button: MouseButton, locationInView: CGPoint)
}

enum MouseMoveEvent {
    case Move (locationInView: CGPoint?, relative: CGPoint)
    case Drag (locationInView: CGPoint?, relative: CGPoint, button: MouseButton)
    case Scroll (scroll: CGPoint)
}

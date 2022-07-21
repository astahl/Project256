//
//  PlatformInput.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation
import GameController
import Combine



extension GameButton {
    mutating func press() {
        self.pressed(true)
    }

    mutating func release() {
        self.pressed(false)
    }

    mutating func pressed(_ down: Bool) {
        self.transitionCount += 1
        self.endedDown = down
    }
}

extension Axis2 {
    mutating func digitalToAnalog() {
        self.end.y = self.up.endedDown ? 1 : self.down.endedDown ? -1 : 0
        self.end.x = self.right.endedDown ? 1 : self.left.endedDown ? -1 : 0
    }

    mutating func analogToDigital(deadZone: Float) {
        if self.end.y > deadZone {
            if abs(self.end.x) < self.end.y {
                self.up.press()
            } else {
                if (self.end.x > deadZone) {
                    self.right.press()
                } else if (self.end.x < -deadZone) {
                    self.left.press()
                }
            }
        } else if self.end.y < -deadZone {
            if abs(self.end.x) < abs(self.end.y) {
                self.down.press()
            } else {
                if (self.end.x > deadZone) {
                    self.right.press()
                } else if (self.end.x < -deadZone) {
                    self.left.press()
                }
            }
        }
    }
}


extension GameInput {
    mutating func appendText(text: String) {
        let cString = text.utf8CString
        let offset = Int(self.textLength);
        let count = min(Int(InputMaxTextLength) - offset, Int(cString.count - 1))
        withUnsafeMutableElementPointer(firstElement: &self.text_utf8.0, offset: offset) {
            ptr in
            cString.withUnsafeBufferPointer {
                cStrPtr in
                ptr.assign(from: cStrPtr.baseAddress!, count: count)
            }
        }
        self.textLength += UInt32(count)
    }
}


extension Mouse {
    mutating func move(relative: CGPoint, position: CGPoint?)
    {
        self.relativeMovement = Vec2f(x: Float(relative.x), y: Float(relative.y))
        if let pos = position {
            self.endedOver = true
            let offset = min(Int(self.trackLength), Int(InputMouseMaxTrackLength - 1))
            withUnsafeMutableElementPointer(firstElement: &self.track.0, offset: offset) {
                ptr in
                ptr.pointee = Vec2f(x: Float(pos.x), y: Float(pos.y))
            }
            self.trackLength += 1
        } else {
            self.endedOver = false
        }
    }
}

class PlatformInput {

    var frameNumber: UInt64 = 0
    var upTime_microseconds: Int64 = 0
    var controllerSubscription: AnyCancellable?
    var settings: GameSettings
    #if os(macOS)
    var keyboardEvents: [KeyboardEvent] = []
    var mouseMoveEvents: [MouseMoveEvent] = []
    var mouseClickEvents: [MouseClickEvent] = []
    #endif
    init(settings: GameSettings) {
        self.settings = settings

    }

    func updateGameInput(gameInput: inout GameInput, frameTime: (microseconds: Int64, seconds: Double)) {
        if frameNumber == 0 {
            setupControllers()
        }

        if let keyboard = GCKeyboard.coalesced?.keyboardInput {
            gameInput.controllers.0.isConnected = true
            if keyboard.isAnyKeyPressed {
                gameInput.controllers.0.isActive = true
            }
            gameInput.controllers.0.stickLeft.up.pressed(keyboard.button(forKeyCode: GCKeyCode.keyW)?.isPressed ?? false);
            gameInput.controllers.0.stickLeft.left.pressed(keyboard.button(forKeyCode: GCKeyCode.keyA)?.isPressed ?? false);
            gameInput.controllers.0.stickLeft.down.pressed(keyboard.button(forKeyCode: GCKeyCode.keyS)?.isPressed ?? false);
            gameInput.controllers.0.stickLeft.right.pressed(keyboard.button(forKeyCode: GCKeyCode.keyD)?.isPressed ?? false);
            gameInput.controllers.0.stickLeft.digitalToAnalog();
        }

        if let mouse = GCMouse.current?.mouseInput {
            gameInput.controllers.0.isConnected = true
            gameInput.controllers.0.shoulderRight.pressed(mouse.leftButton.isPressed)
        }

        gameInput.frameNumber = self.frameNumber
        self.frameNumber += 1
        gameInput.upTime_microseconds = self.upTime_microseconds
        self.upTime_microseconds += Int64(Double(frameTime.microseconds) * settings.timeScale)
        gameInput.elapsedTime_s = frameTime.seconds * settings.tickScale

#if os(macOS)
        for key in keyboardEvents {
            switch key {
            case .Down(_, let characters?, let modifiers):
                gameInput.appendText(text: characters)
                print(modifiers)
            default: break
            }
        }

        for move in mouseMoveEvents {
            switch move {
            case .Move(let position, let relative):
                gameInput.mouse.move(relative: relative, position: position)
            case .Drag(let position, let relative, _):
                gameInput.mouse.move(relative: relative, position: position)
            case .Scroll(_):
                break
            }
        }

        for mouseClickEvent in mouseClickEvents {
            switch mouseClickEvent {
            case .Down(.Left, _):
                gameInput.mouse.buttonLeft.press()
            case .Down(.Right, _):
                gameInput.mouse.buttonRight.press()
            case .Down(.Other, _):
                gameInput.mouse.buttonMiddle.press()
            case .Up(.Left, _):
                gameInput.mouse.buttonLeft.release()
            case .Up(.Right, _):
                gameInput.mouse.buttonRight.release()
            case .Up(.Other, _):
                gameInput.mouse.buttonMiddle.release()

            }
        }

        keyboardEvents.removeAll()
        mouseMoveEvents.removeAll()
        mouseClickEvents.removeAll()
#endif
    }

    func setupControllers() {

        if let keyboard = GCKeyboard.coalesced {
            print(keyboard.vendorName ?? "")
//            if input.controllers.0.subType.rawValue == 0 {
//                input.controllers.0.subType = .init(rawValue: 1) // WTF why doesnt the enum work
//            }
//            if input.controllers.0.subType.rawValue == 2 {
//                input.controllers.0.subType = .init(rawValue: 3)
//            }
//            if (input.controllerCount == 0) {
//                input.controllerCount = 1
//                input.controllers.0.isConnected = true
//            }
//            if let keyboardInput = keyboard.keyboardInput {
//                if keyboardInput.keyChangedHandler == nil {
//                    keyboardInput.keyChangedHandler = keyChanged(keyboard:key:keyCode:pressed:)
//                }
//            }
        }

        if let mouse = GCMouse.current {
            print(mouse.vendorName ?? "")
//            if input.controllers.0.subType.rawValue == 0 {
//                input.controllers.0.subType = .init(rawValue: 2) // WTF why doesnt the enum work
//            }
//            if input.controllers.0.subType.rawValue == 1 {
//                input.controllers.0.subType = .init(rawValue: 3)
//            }
//            if (input.controllerCount == 0) {
//                input.controllerCount = 1
//                input.controllers.0.isConnected = true
//            }
//            if let mouseInput = mouse.mouseInput {
//                if mouseInput.mouseMovedHandler == nil {
//                    mouseInput.mouseMovedHandler = mouseMoved(mouse:deltaX:deltaY:)
//                }
//                mouseInput.leftButton.valueChangedHandler = { self.input.controllers.0.buttonA.pressed($2) }
//                mouseInput.rightButton?.valueChangedHandler = { self.input.controllers.0.buttonB.pressed($2) }
//                mouseInput.middleButton?.valueChangedHandler = { self.input.controllers.0.buttonStickRight.pressed($2) }
//            }
        }
    }

    func keyChanged(keyboard: GCKeyboardInput, key: GCControllerButtonInput, keyCode: GCKeyCode, pressed: Bool)
    {
//        withUnsafeMutableElementPointer(firstElement: &input.controllers.0, offset: 0) {
//            controller in
//            switch keyCode {
//            case .escape: controller.pointee.buttonBack.pressed(pressed)
//            case .keyW, .upArrow: controller.pointee.stickLeft.up.pressed(pressed)
//            case .keyA, .leftArrow: controller.pointee.stickLeft.left.pressed(pressed)
//            case .keyS, .downArrow: controller.pointee.stickLeft.down.pressed(pressed)
//            case .keyD, .rightArrow: controller.pointee.stickLeft.right.pressed(pressed)
//            default: break
//            }
//            controller.pointee.stickLeft.digitalToAnalog()
//            controller.pointee.stickLeft.latches = true
//            controller.pointee.isActive = true
//        }
    }


    func mouseMoved(mouse: GCMouseInput, deltaX: Float, deltaY: Float)
    {
//        input.controllers.0.stickRight.end.x = deltaX
//        input.controllers.0.stickRight.end.y = deltaY
//        input.controllers.0.stickRight.analogToDigital(deadZone: 0.2)
//        input.controllers.0.isActive = true
    }

#if os(macOS)
    func pushKeyboardEvent(_ key: KeyboardEvent) {
        keyboardEvents.append(key)
    }

    func pushMouseMoveEvent(_ move: MouseMoveEvent) {
        mouseMoveEvents.append(move)
    }

    func pushMouseClickEvent(_ click: MouseClickEvent) {
        mouseClickEvents.append(click)
    }
#endif
}


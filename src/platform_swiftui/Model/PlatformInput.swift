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
        if self.endedDown != down {
            self.transitionCount += 1
            self.endedDown = down
        }
    }
}


extension Axis2 {
    mutating func digitalToAnalog() {
        self.end.y = self.up.endedDown ? 1 : self.down.endedDown ? -1 : 0
        self.end.x = self.right.endedDown ? 1 : self.left.endedDown ? -1 : 0
    }

    mutating func analogToDigital(deadZone: Float) {
        self.left.pressed(self.end.x < -deadZone)
        self.right.pressed(self.end.x > deadZone)
        self.down.pressed(self.end.y < -deadZone)
        self.up.pressed(self.end.y > deadZone)
    }
}

extension GCControllerButtonInput {
    func mapToInputButton(_ inputButton: UnsafeMutablePointer<GameButton>) {
        self.pressedChangedHandler = {
            button, value, pressed in
            inputButton.pointee.pressed(pressed)
        }
    }

    func mapToInputAxis1(_ inputAxis1: UnsafeMutablePointer<Axis1>) {
        self.valueChangedHandler = {
            button, value, pressed in
            inputAxis1.pointee.end = value
            inputAxis1.pointee.trigger.pressed(pressed)
            inputAxis1.pointee.isAnalog = true
        }
    }
}


extension GCControllerDirectionPad {
    func mapToInputAxis2(_ inputAxis2: UnsafeMutablePointer<Axis2>) {
        self.valueChangedHandler = {
            dpad, x, y in
            inputAxis2.pointee.end.x = x
            inputAxis2.pointee.end.y = y
            inputAxis2.pointee.analogToDigital(deadZone: 0.5)
            inputAxis2.pointee.isAnalog = dpad.isAnalog
            inputAxis2.pointee.latches = dpad.isAnalog
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
    var currentGameInput: UnsafeMutablePointer<GameInput>
    var frameNumber: UInt64 = 0
    var upTime_microseconds: Int64 = 0
    var controllerSubscription: AnyCancellable?
    var subscriptions = Set<AnyCancellable>()
    var settings: GameSettings
    #if os(macOS)
    var keyboardEvents: [KeyboardEvent] = []
    var mouseMoveEvents: [MouseMoveEvent] = []
    var mouseClickEvents: [MouseClickEvent] = []
    #endif
    init(settings: GameSettings, input: UnsafeMutablePointer<GameInput>) {
        self.currentGameInput = input
        self.settings = settings
        subscriptions.insert(NotificationCenter.default.publisher(for: Notification.Name.GCKeyboardDidConnect)
            .sink {
                if let keyboard = $0.object as? GCKeyboard {
                    print("keyboard: \(keyboard)" )
                }
            })
        subscriptions.insert(NotificationCenter.default.publisher(for: Notification.Name.GCMouseDidBecomeCurrent)
            .sink { self.onCurrentMouse(gcMouse: $0.object as! GCMouse) })
        subscriptions.insert(NotificationCenter.default.publisher(for: Notification.Name.GCControllerDidBecomeCurrent)
            .sink { self.onCurrentController(gcController: $0.object as! GCController) })
    }


    func onCurrentController(gcController: GCController) {
        if let extendedGamepad = gcController.extendedGamepad {
            withController(index: 2) {
                controller in

                extendedGamepad.buttonA.mapToInputButton(&controller.buttonA)
                extendedGamepad.buttonB.mapToInputButton(&controller.buttonB)
                extendedGamepad.buttonX.mapToInputButton(&controller.buttonX)
                extendedGamepad.buttonY.mapToInputButton(&controller.buttonY)
                extendedGamepad.leftShoulder.mapToInputButton(&controller.shoulderLeft)
                extendedGamepad.rightShoulder.mapToInputButton(&controller.shoulderRight)
                extendedGamepad.buttonOptions?.mapToInputButton(&controller.buttonBack)
                extendedGamepad.buttonMenu.mapToInputButton(&controller.buttonStart)
                extendedGamepad.leftThumbstickButton?.mapToInputButton(&controller.buttonStickLeft)
                extendedGamepad.rightThumbstickButton?.mapToInputButton(&controller.buttonStickRight)
                extendedGamepad.leftTrigger.mapToInputAxis1(&controller.triggerLeft)
                extendedGamepad.rightTrigger.mapToInputAxis1(&controller.triggerRight)
                extendedGamepad.leftThumbstick.mapToInputAxis2(&controller.stickLeft)
                extendedGamepad.rightThumbstick.mapToInputAxis2(&controller.stickRight)
                extendedGamepad.dpad.mapToInputAxis2(&controller.dPad)
                controller.isConnected = true
                controller.isActive = true
            }
        }
    }

    func onCurrentMouse(gcMouse: GCMouse) {
        gcMouse.mouseInput?.mouseMovedHandler = {
            mouseInput, x, y in
            self.withKbmController {
                kbmController in
                kbmController.stickRight.end.x += x
                kbmController.stickRight.end.y += y
                kbmController.stickRight.isAnalog = true
                kbmController.stickRight.analogToDigital(deadZone: 1)
            }
        }
    }




    func updateGameInput(frameTime: (microseconds: Int64, seconds: Double)) {
        withKbmController() {
            kbmController in
            if let keyboard = GCKeyboard.coalesced?.keyboardInput {
                kbmController.isConnected = true
                if keyboard.isAnyKeyPressed {
                    kbmController.isActive = true
                }
                kbmController.stickLeft.up.pressed(keyboard.button(forKeyCode: GCKeyCode.keyW)?.isPressed ?? false);
                kbmController.stickLeft.left.pressed(keyboard.button(forKeyCode: GCKeyCode.keyA)?.isPressed ?? false);
                kbmController.stickLeft.down.pressed(keyboard.button(forKeyCode: GCKeyCode.keyS)?.isPressed ?? false);
                kbmController.stickLeft.right.pressed(keyboard.button(forKeyCode: GCKeyCode.keyD)?.isPressed ?? false);
                kbmController.stickLeft.digitalToAnalog();
            }

            if let mouse = GCMouse.current?.mouseInput {
                kbmController.isConnected = true
                kbmController.shoulderRight.pressed(mouse.leftButton.isPressed)
            }
        }

        currentGameInput.pointee.frameNumber = self.frameNumber
        self.frameNumber += 1
        currentGameInput.pointee.upTime_microseconds = self.upTime_microseconds
        self.upTime_microseconds += Int64(Double(frameTime.microseconds) * settings.timeScale)
        currentGameInput.pointee.elapsedTime_s = frameTime.seconds * settings.tickScale

#if os(macOS)
        for key in keyboardEvents {
            switch key {
            case .Down(_, let characters?, let modifiers):
                currentGameInput.pointee.appendText(text: characters)
                print(modifiers)
            default: break
            }
        }

        for move in mouseMoveEvents {
            switch move {
            case .Move(let position, let relative):
                currentGameInput.pointee.mouse.move(relative: relative, position: position)
            case .Drag(let position, let relative, _):
                currentGameInput.pointee.mouse.move(relative: relative, position: position)
            case .Scroll(_):
                break
            }
        }

        for mouseClickEvent in mouseClickEvents {
            switch mouseClickEvent {
            case .Down(.Left, _):
                currentGameInput.pointee.mouse.buttonLeft.press()
            case .Down(.Right, _):
                currentGameInput.pointee.mouse.buttonRight.press()
            case .Down(.Other, _):
                currentGameInput.pointee.mouse.buttonMiddle.press()
            case .Up(.Left, _):
                currentGameInput.pointee.mouse.buttonLeft.release()
            case .Up(.Right, _):
                currentGameInput.pointee.mouse.buttonRight.release()
            case .Up(.Other, _):
                currentGameInput.pointee.mouse.buttonMiddle.release()

            }
        }

        keyboardEvents.removeAll()
        mouseMoveEvents.removeAll()
        mouseClickEvents.removeAll()
#endif
    }

    func withController(index: Int, action: @escaping ((inout P256GameController) -> Void))
    {
        withUnsafeMutableElementPointer(firstElement: &currentGameInput.pointee.controllers.0, offset: index) {
            action(&$0.pointee)
        }
    }

    func withKbmController(action: @escaping ((inout P256GameController) -> Void))
    {
        withUnsafeMutableElementPointer(firstElement: &currentGameInput.pointee.controllers.0, offset: 0) {
            action(&$0.pointee)
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


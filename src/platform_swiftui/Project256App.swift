//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI

@main
struct Project256App: App {
    @StateObject var gameState = GameState()
    @State var letterboxColor = Color.red
    private var timer: Timer?

    var body: some Scene {
        WindowGroup {
            ZStack {
            MetalView(drawBuffer: gameState.drawBuffer)
                .letterboxColor(self.letterboxColor)
                .mouseMove {
                    relative, position in
                    inputPushMouseTrack(&gameState.input, Float(position.x), Float(position.y))
                }
                Text("\(gameState.input.mouse.track.0.x), \(gameState.input.mouse.track.0.y)")
            }
        }
        #if os(macOS)
        Settings {
            VStack {
                ColorPicker("Letterbox", selection: $letterboxColor)
            }.padding()
        }
        #endif
    }
}

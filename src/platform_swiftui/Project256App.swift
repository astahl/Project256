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
    @State var scalingMode = ScalingMode.ScalingFull
    @State var letterboxColor = Color.red
    private var timer: Timer?

    var body: some Scene {
        WindowGroup {
            ZStack {
            MetalView(scalingMode: scalingMode, drawBuffer: gameState.drawBuffer)
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
                Picker("Scaling Mode", selection: $scalingMode) {
                    Text("Full").tag(ScalingMode.ScalingFull)
                    Text("Integer").tag(ScalingMode.ScalingInteger)
                }
                ColorPicker("Letterbox", selection: $letterboxColor)
            }.padding()
        }
        #endif
    }
}

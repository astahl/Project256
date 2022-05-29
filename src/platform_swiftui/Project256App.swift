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
    private var timer: Timer?

    var body: some Scene {
        WindowGroup {
            MetalView(drawBuffer: gameState.drawBuffer, scalingMode: scalingMode)
        }
        #if os(macOS)
        Settings {
            VStack {
                Picker("Scaling Mode", selection: $scalingMode) {
                    Text("Full").tag(ScalingMode.ScalingFull)
                    Text("Integer").tag(ScalingMode.ScalingInteger)
                }
            }.padding()
        }
        #endif
    }
}

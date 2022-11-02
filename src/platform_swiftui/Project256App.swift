//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI
import Combine




 @main
struct Project256App: App {
    class AppSubscriptions {
        var profiling: AnyCancellable? = nil
        var highfrequency: AnyCancellable? = nil
    }

    @State var profilingString = String()
    @State var gameState: GameState
    @State var gameSettings: GameSettings
    @State var showProfilingView = false

    var subscriptions: AppSubscriptions

    init() {
        let settings = GameSettings()
        gameSettings = settings
        gameState = GameState(settings: settings)
        subscriptions = AppSubscriptions()
    }

    func doTick(_ _: Date) {
        gameState.tick()
    }

    func startSubscriptions() {
        resetTickSubscription(tickTargetHz: gameSettings.tickTargetHz)
        resetProfilingSubscription(isProfiling: self.showProfilingView)
    }

    func resetProfilingSubscription(isProfiling: Bool) {
        self.subscriptions.profiling?.cancel()
        if isProfiling {
            self.subscriptions.profiling = Timer.publish(every: 1.0, on: .main, in: .default)
                .autoconnect()
                .sink {
                    date in
                    PlatformProfiling.withInstance {
                        profiling in
                        profilingString = String(unsafeUninitializedCapacity: 1000) {
                            buffer in
                            return Int(profiling.timingData.printTo(buffer: buffer.baseAddress!, size: Int32(buffer.count)))
                        }
                        profiling.timingData.clear()
                    }
                }
        }
    }

    func resetTickSubscription(tickTargetHz: Double)
    {
        self.subscriptions.highfrequency?.cancel()
        if (tickTargetHz != 0) {
            self.subscriptions.highfrequency = Timer.publish(every: 1 / tickTargetHz, on: .main, in: .common)
                .autoconnect()
                .sink(receiveValue: self.doTick)
        }
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
                GameView(state: gameState)
                    .onAppear {
                        startSubscriptions()
                    }
                    .onDisappear {
                        // just quit when window is closed
                        exit(EXIT_SUCCESS)
                    }
                    //.background(.linearGradient(.init(colors: [Color.cyan, Color.purple]), startPoint: .topLeading, endPoint: .bottomTrailing))

                //.overlay(Ellipse().foregroundColor(.gray).opacity(0.3).blur(radius: 100))
                    //.ignoresSafeArea()

                if self.showProfilingView {
                    ProfilingView(profilingString: profilingString)
                }
            }
        }
        .commands {
            // remove new window command
            CommandGroup(replacing: .newItem) {
            }
        }
        .onChange(of: gameSettings.tickTargetHz, perform: resetTickSubscription(tickTargetHz:))
        .onChange(of: self.showProfilingView, perform: resetProfilingSubscription(isProfiling:))

        #if os(macOS)
        Settings {
            List {
                Slider(value: $gameSettings.tickScale, in: 0...5, step: 0.1) {
                    Text("Tick Scale \(gameSettings.tickScale)")
                } minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("5")
                }
                Slider(value: $gameSettings.timeScale, in: 0...5, step: 0.1) {
                    Text("Time Scale \(gameSettings.timeScale)")
                }  minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("5")
                }
                Slider(value: $gameSettings.tickTargetHz, in: 0...200, step: 5) {
                    Text("Tick Hz \(gameSettings.tickTargetHz)")
                } minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("200")
                }
                Picker("FPS Target", selection: $gameSettings.frameTargetHz) {
                    ForEach(FPSTargets.allCases) {
                        fps in
                        Text(fps.rawValue.formatted())
                    }
                }
                Toggle("Profiling", isOn: $showProfilingView)
            }.padding()
        }
        #endif
    }
}

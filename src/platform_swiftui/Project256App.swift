//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI
import Combine


#if os(macOS)
func setCursorVisible(_ shouldShow: Bool, currentlyHidden: Bool) -> Bool
{
    if shouldShow && currentlyHidden {
        CGDisplayShowCursor(CGMainDisplayID())
        return false
    } else if !shouldShow && !currentlyHidden {
        CGDisplayHideCursor(CGMainDisplayID())
        return true
    }
    return currentlyHidden
}
#endif

enum FPSTargets : Int, CaseIterable, Identifiable {
    case Stop = 0, _5 = 5, _15 = 15, _60 = 60, _120 = 120
    var id: Self { self }
}

@main
struct Project256App: App {
    class AppSubscriptions {
        var profiling: AnyCancellable? = nil
        var highfrequency: AnyCancellable? = nil
    }

    @State var profilingString: String = .init()
    @State var gameState: GameState
    @State var gameSettings: GameSettings

    var profilingBuffer = UnsafeMutableBufferPointer<CChar>.allocate(capacity: 1000)
    var subscriptions: AppSubscriptions

    init() {
        gameState = GameState()
        subscriptions = AppSubscriptions()
        gameSettings = GameSettings()
    }

    func doTick(_ _: Date) {
        gameState.tick(settings: gameSettings)
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
                GameView(state: gameState, settings: gameSettings)
                .onAppear {
                    self.subscriptions.highfrequency = Timer.publish(every: 1 / gameSettings.tickTargetHz, on: .main, in: .common)
                        .autoconnect()
                        .sink(receiveValue: self.doTick)
                    self.subscriptions.profiling = Timer.publish(every: 1.0, on: .main, in: .default)
                        .autoconnect()
                        .sink {
                            date in
                            let length = GameState.timingData?.printTo(buffer: profilingBuffer.baseAddress!, size: Int32(profilingBuffer.count))
                            profilingString = String.init(bytesNoCopy: profilingBuffer.baseAddress!, length: Int(length!), encoding: .ascii, freeWhenDone: false)!

                            GameState.timingData?.clear()
                        }
                }
                .onDisappear {
                    self.subscriptions.profiling?.cancel()
                    self.subscriptions.highfrequency?.cancel()
                }
                .background(.linearGradient(.init(colors: [Color.cyan, Color.purple]), startPoint: .topLeading, endPoint: .bottomTrailing))

                //.overlay(Ellipse().foregroundColor(.gray).opacity(0.3).blur(radius: 100))
                //.ignoresSafeArea()
                .onChange(of: gameSettings.tickTargetHz, perform: {
                    newTickTarget in

                    self.subscriptions.highfrequency?.cancel()
                    if (newTickTarget != 0) {
                        self.subscriptions.highfrequency = Timer.publish(every: 1 / newTickTarget, on: .main, in: .common)
                            .autoconnect()
                            .sink(receiveValue: self.doTick)
                    }
                })
                HStack {
                    Text(profilingString)
                        .font(.body.monospaced())
                        .multilineTextAlignment(.leading)
                        .shadow(radius: 5)
                        .padding()
                    Spacer()
                }
            }
        }
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
//                (value: $gameState.frameTargetHz, in: 0...120, step: 5) {
//                    Text("FPS Target \(gameState.tickTargetHz)")
//                } minimumValueLabel: {
//                    Text("0")
//                } maximumValueLabel: {
//                    Text("120")
//                }
            }.padding()
        }
        #endif
    }
}

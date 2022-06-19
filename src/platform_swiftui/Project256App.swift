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

    var profilingBuffer = UnsafeMutableBufferPointer<CChar>.allocate(capacity: 1000)
    var subscriptions: AppSubscriptions

    init() {
        gameState = GameState()
        subscriptions = AppSubscriptions()
    }

    func doTick(_ _: Date) {
        gameState.tick()
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
                GameView(gameState: gameState)
                .onAppear {
                    self.subscriptions.highfrequency = Timer.publish(every: 1 / gameState.tickTargetHz, on: .main, in: .common)
                        .autoconnect()
                        .sink(receiveValue: self.doTick)
                    self.subscriptions.profiling = Timer.publish(every: 1.0, on: .main, in: .default)
                        .autoconnect()
                        .sink {
                            date in
                            let length = profiling_time_print(&GameState.timingData, profilingBuffer.baseAddress!, Int32(profilingBuffer.count))
                            profilingString = String.init(bytesNoCopy: profilingBuffer.baseAddress!, length: Int(length), encoding: .ascii, freeWhenDone: false)!

                            profiling_time_clear(&GameState.timingData)
                        }
                }
                .onDisappear {
                    self.subscriptions.profiling?.cancel()
                    self.subscriptions.highfrequency?.cancel()
                }
                .background(.linearGradient(.init(colors: [Color.cyan, Color.purple]), startPoint: .topLeading, endPoint: .bottomTrailing))
                .overlay(Ellipse().foregroundColor(.gray).opacity(0.3).blur(radius: 100))
                .ignoresSafeArea()
                .onChange(of: gameState.tickTargetHz, perform: {
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
                Slider(value: $gameState.tickScale, in: 0...5, step: 0.1) {
                    Text("Tick Scale \(gameState.tickScale)")
                } minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("5")
                }
                Slider(value: $gameState.timeScale, in: 0...5, step: 0.1) {
                    Text("Time Scale \(gameState.timeScale)")
                }  minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("5")
                }
                Slider(value: $gameState.tickTargetHz, in: 0...200, step: 5) {
                    Text("Tick Hz \(gameState.tickTargetHz)")
                } minimumValueLabel: {
                    Text("0")
                } maximumValueLabel: {
                    Text("200")
                }
                Picker("FPS Target", selection: $gameState.frameTargetHz) {
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

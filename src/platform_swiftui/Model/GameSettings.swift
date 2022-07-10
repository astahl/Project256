//
//  GameSettings.swift
//  Project256
//
//  Created by Andreas Stahl on 25.06.22.
//

import Foundation


enum FPSTargets : Int, CaseIterable, Identifiable {
    case Stop = 0, _5 = 5, _15 = 15, _60 = 60, _120 = 120
    var id: Self { self }
}


class GameSettings {
    @Published var tickScale: Double = 1.0
    @Published var timeScale: Double = 1.0
    @Published var tickTargetHz: Double = 100.0
    @Published var frameTargetHz: FPSTargets = ._60
}

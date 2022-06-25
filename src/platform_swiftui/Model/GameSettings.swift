//
//  GameSettings.swift
//  Project256
//
//  Created by Andreas Stahl on 25.06.22.
//

import Foundation

class GameSettings {
    @Published var tickScale: Double = 1.0
    @Published var timeScale: Double = 1.0
    @Published var tickTargetHz: Double = 100.0
    @Published var frameTargetHz: FPSTargets = ._60
}

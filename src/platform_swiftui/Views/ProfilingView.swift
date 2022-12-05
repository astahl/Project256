//
//  ProfilingView.swift
//  Project256
//
//  Created by Andreas Stahl on 02.11.22.
//

import SwiftUI

struct ProfilingView: View {
    var profilingString: String

    var body: some View {
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

struct ProfilingView_Previews: PreviewProvider {
    static var previews: some View {
        ProfilingView(profilingString: "Hallo")
    }
}

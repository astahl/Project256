//
//  StructHelpers.h
//  Project256
//
//  Created by Andreas Stahl on 01.06.22.
//

#ifndef StructHelpers_h
#define StructHelpers_h

#ifdef CXX
extern "C" {
#endif

void inputPushMouseTrack(struct GameInput* input, float x, float y);
void inputClear(struct GameInput* input);

#ifdef CXX
}
#endif
#endif /* StructHelpers_hpp */

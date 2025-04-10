/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2025                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __OPENSPACE_CORE___MOUSEINPUTSTATE___H__
#define __OPENSPACE_CORE___MOUSEINPUTSTATE___H__

#include <openspace/util/mouse.h>
#include <ghoul/glm.h>
#include <vector>

namespace openspace::interaction {

// This class represents the global input state of interaction devices
class MouseInputState {
public:
    // Callback functions
    void mouseButtonCallback(MouseButton button, MouseAction action);
    void mousePositionCallback(double mouseX, double mouseY);
    void mouseScrollWheelCallback(double mouseScrollDelta);

    // Accessors
    const std::vector<MouseButton>& pressedMouseButtons() const;
    glm::dvec2 mousePosition() const;
    double mouseScrollDelta() const;
    bool isMouseButtonPressed(MouseButton mouseButton) const;

private:
    // Input from mouse
    std::vector<MouseButton> _mouseButtonsDown;
    glm::dvec2 _mousePosition = glm::dvec2(0.0);
    double _mouseScrollDelta = 0.0;
};

} // namespace openspace::interaction

#endif // __OPENSPACE_CORE___MOUSEINPUTSTATE___H__

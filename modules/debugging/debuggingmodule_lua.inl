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

#include <openspace/scene/scene.h>
#include <ghoul/lua/lua_helper.h>

namespace {

constexpr const char RenderedPathIdentifier[] = "CurrentCameraPath";
constexpr const char RenderedPointsIdentifier[] = "CurrentPathControlPoints";
constexpr const char DebuggingGuiPath[] = "/Debugging";

constexpr glm::vec3 PathColor = glm::vec3(1.0, 1.0, 0.0);
constexpr glm::vec3 OrientationLineColor = glm::vec3(0.0, 1.0, 1.0);

/**
 * Render the current camera path from the path navigation system. The first optional
 * argument is the number of samples to take along the path (defaults to 100). If a second
 * optional argument is included and set to true, a line indicating the camera view
 * direction along the path will also be rendered. This can be useful when debugging
 * camera orientations. Finally, the third optional argument can be used to set the length
 * (in meter) of the view direction lines.
 */
[[codegen::luawrap]] void renderCameraPath(int nSteps = 100,
                                           bool renderDirections = false,
                                           double directionLineLength = 6e7)
{
    using namespace openspace;

    if (!global::navigationHandler->pathNavigator().hasCurrentPath()) {
        LWARNINGC("Debugging: PathNavigation", "There is no current path to render");
    }

    const interaction::Path* currentPath =
        global::navigationHandler->pathNavigator().currentPath();

    // Parent node. Note that we only render one path at a time, so remove the previously
    // rendered one, if any
    std::string addParentScript = std::format(
        "if openspace.hasSceneGraphNode('{0}') then "
            "openspace.removeSceneGraphNode('{0}') "
        "end "
        "openspace.addSceneGraphNode({{ Identifier = '{0}' }})",
        RenderedPathIdentifier
    );

    global::scriptEngine->queueScript(addParentScript);

    // Get the poses along the path
    std::vector<CameraPose> poses;
    const double du = 1.0 / nSteps;
    const double length = currentPath->pathLength();
    for (double u = 0.0; u < 1.0; u += du) {
        const CameraPose p = currentPath->interpolatedPose(u * length);
        poses.push_back(p);
    }
    poses.push_back(currentPath->endPoint().pose());

    // Create node lines between the positions
    auto pointIdentifier = [](int i) {
        return std::format("Point_{}", i);
    };

    auto addPoint = [](const std::string& id, glm::dvec3 p) {
        const std::string pointNode = "{"
            "Identifier = '" + id + "',"
            "Parent = '" + RenderedPathIdentifier + "',"
            "Transform = { "
                "Translation = {"
                    "Type = 'StaticTranslation',"
                    "Position = " + ghoul::to_string(p) + ""
                "},"
            "}"
        "}";

        global::scriptEngine->queueScript(
            std::format("openspace.addSceneGraphNode({})", pointNode)
        );
    };

    auto addLineBetweenPoints = [](const std::string& id1, const std::string& id2,
                                   const glm::vec3& color, float lineWidth)
    {
        const std::string lineNode = "{"
            "Identifier = '" + std::format("Line{}", id1) + "',"
            "Parent = '" + RenderedPathIdentifier + "',"
            "Renderable = {"
                "Enabled = true,"
                "Type = 'RenderableNodeLine',"
                "StartNode = '" + id1 + "',"
                "EndNode = '" + id2 + "',"
                "LineWidth = " + std::to_string(lineWidth) + ","
                "Color = " + ghoul::to_string(color) + ""
            "}"
        "}";

        global::scriptEngine->queueScript(
            std::format("openspace.addSceneGraphNode({})", lineNode)
        );
    };

    auto addDirectionLine = [addPoint, addLineBetweenPoints]
                            (const std::string& pointId, const CameraPose& p,
                             double lineLength)
    {
        const glm::dvec3 dir = glm::normalize(p.rotation * glm::dvec3(0.0, 0.0, -1.0));
        const glm::dvec3 pointPosition = p.position + lineLength * dir;
        const std::string id = std::format("{}_orientation", pointId);

        addPoint(id, pointPosition);
        addLineBetweenPoints(id, pointId, OrientationLineColor, 2.f);
    };

    // Add first point separately so that we can create first line in for loop
    addPoint(pointIdentifier(0), poses.front().position);
    if (renderDirections) {
        addDirectionLine(pointIdentifier(0), poses.front(), directionLineLength);
    }

    for (int i = 1; i < static_cast<int>(poses.size()); i++) {
        addPoint(pointIdentifier(i), poses[i].position);
        addLineBetweenPoints(pointIdentifier(i), pointIdentifier(i - 1), PathColor, 4.f);

        if (renderDirections) {
            addDirectionLine(pointIdentifier(i), poses[i], directionLineLength);
        }
    }
}

// Removes the currently rendered camera path if there is one.
[[codegen::luawrap]] void removeRenderedCameraPath() {
    using namespace openspace;
    const std::string script = std::format(
        "openspace.removeSceneGraphNode('{}');", RenderedPathIdentifier
    );
    global::scriptEngine->queueScript(script);
}

/**
 * Render the control points for the camera path spline as spheres. The optional argument
 * can be used to set the radius of the created spheres.
 */
[[codegen::luawrap]] void renderPathControlPoints(double radius = 2000000.0) {
    using namespace openspace;

    if (!global::navigationHandler->pathNavigator().hasCurrentPath()) {
        LWARNINGC(
            "Debugging: PathNavigation", "There is no current path to sample points from"
        );
    }

    const interaction::Path* currentPath =
        global::navigationHandler->pathNavigator().currentPath();

    // Parent node. Note that we only render one set of points at a time, so remove any
    // previously rendered ones
    std::string addParentScript = std::format(
        "if openspace.hasSceneGraphNode('{0}') then "
            "openspace.removeSceneGraphNode('{0}') "
        "end "
        "openspace.addSceneGraphNode({{ Identifier = '{0}' }})",
        RenderedPointsIdentifier
    );

    global::scriptEngine->queueScript(addParentScript);

    const std::vector<glm::dvec3> points = currentPath->controlPoints();

    const std::string guiPath =
        std::format("{}/Camera Path Control Points", DebuggingGuiPath);

    const char* colorTexturePath = "openspace.absPath("
        "openspace.createSingleColorImage('point_color', { 0.0, 1.0, 0.0 })"
        ")";

    for (size_t i = 0; i < points.size(); i++) {
        const std::string& node = "{"
            "Identifier = 'ControlPoint_" + std::to_string(i) + "',"
            "Parent = '" + RenderedPointsIdentifier + "',"
            "Transform = { "
                "Translation = {"
                    "Type = 'StaticTranslation',"
                    "Position = " + ghoul::to_string(points[i]) + ""
                "},"
            "},"
            "Renderable = {"
                "Type = 'RenderableSphere',"
                "Enabled = true,"
                "Segments = 30,"
                "Size = " + std::to_string(radius) + ","
                "Texture = " + colorTexturePath + ""
            "},"
            "GUI = {"
                "Name = 'Control Point " + std::to_string(i) + "',"
                "Path = '" + guiPath + "'"
            "}"
        "}";

        global::scriptEngine->queueScript(
            std::format("openspace.addSceneGraphNode({})", node)
        );
    }
}

// Removes the rendered control points.
[[codegen::luawrap]] void removePathControlPoints() {
    using namespace openspace;
    const std::string script = std::format(
        "openspace.removeSceneGraphNode('{}');", RenderedPointsIdentifier
    );
    global::scriptEngine->queueScript(script);
}

#include "debuggingmodule_lua_codegen.cpp"

} // namespace

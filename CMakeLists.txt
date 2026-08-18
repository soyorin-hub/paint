cmake_minimum_required(VERSION 3.16)
project(Paint VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Svg
    SvgWidgets
)

# ====== Source files ======
set(SOURCES
    src/main.cpp
    # App
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    # Canvas
    src/canvas/CanvasView.h
    src/canvas/CanvasView.cpp
    src/canvas/CanvasScene.h
    src/canvas/CanvasScene.cpp
    # Style
    src/style/ShapeStyle.h
    src/style/ShapeStyle.cpp
    # Shapes
    src/shapes/ShapeBase.h
    src/shapes/ShapeBase.cpp
    src/shapes/RectShape.h
    src/shapes/RectShape.cpp
    src/shapes/EllipseShape.h
    src/shapes/EllipseShape.cpp
    src/shapes/LineShape.h
    src/shapes/LineShape.cpp
    src/shapes/TriangleShape.h
    src/shapes/TriangleShape.cpp
    src/shapes/DiamondShape.h
    src/shapes/DiamondShape.cpp
    src/shapes/ArrowShape.h
    src/shapes/ArrowShape.cpp
    src/shapes/FreehandShape.h
    src/shapes/FreehandShape.cpp
    src/shapes/TextShape.h
    src/shapes/TextShape.cpp
    # Tools
    src/tools/ToolBase.h
    src/tools/ToolBase.cpp
    src/tools/ToolManager.h
    src/tools/ToolManager.cpp
    src/tools/SelectTool.h
    src/tools/SelectTool.cpp
    src/tools/ShapeTool.h
    src/tools/ShapeTool.cpp
    src/tools/FreehandTool.h
    src/tools/FreehandTool.cpp
    src/tools/TextTool.h
    src/tools/TextTool.cpp
    src/tools/EraserTool.h
    src/tools/EraserTool.cpp
    # Layers
    src/layers/Layer.h
    src/layers/Layer.cpp
    src/layers/LayerPanel.h
    src/layers/LayerPanel.cpp
    # Properties
    src/properties/PropertyPanel.h
    src/properties/PropertyPanel.cpp
    # History
    src/history/HistoryPanel.h
    src/history/HistoryPanel.cpp
    # Document
    src/document/Document.h
    src/document/Document.cpp
    src/document/FileManager.h
    src/document/FileManager.cpp
    # Commands
    src/commands/AddShapeCommand.h
    src/commands/AddShapeCommand.cpp
    src/commands/RemoveShapeCommand.h
    src/commands/RemoveShapeCommand.cpp
    src/commands/TransformCommand.h
    src/commands/TransformCommand.cpp
    src/commands/StyleCommand.h
    src/commands/StyleCommand.cpp
    src/commands/LineCommand.h
    src/commands/LineCommand.cpp
    src/commands/TextCommand.h
    src/commands/TextCommand.cpp
    src/commands/ResizeCommand.h
    src/commands/ResizeCommand.cpp
    src/commands/FontCommand.h
    src/commands/FontCommand.cpp
    src/commands/LayerCommand.h
    src/commands/LayerCommand.cpp
    src/commands/RotateCommand.h
    src/commands/RotateCommand.cpp
    src/commands/GroupCommand.h
    src/commands/GroupCommand.cpp
    src/commands/ReorderShapesCommand.h
    src/commands/ReorderShapesCommand.cpp
    src/commands/GroupRenameCommand.h
    src/commands/GroupRenameCommand.cpp
)

# UI files
set(UI_FILES
    src/app/MainWindow.ui
    src/layers/LayerPanel.ui
    src/properties/PropertyPanel.ui
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${UI_FILES}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

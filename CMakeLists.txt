cmake_minimum_required(VERSION 3.21)
project(DocuReaderExchange VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Pdf PdfWidgets)

qt_add_executable(DocuReaderExchange
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(DocuReaderExchange PRIVATE
    Qt6::Widgets
    Qt6::Pdf
    Qt6::PdfWidgets
)

target_compile_definitions(DocuReaderExchange PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

install(TARGETS DocuReaderExchange
    RUNTIME DESTINATION bin
)

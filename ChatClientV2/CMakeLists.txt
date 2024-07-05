cmake_minimum_required(VERSION 3.12)

project(Chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the Boost root directory
set(BOOST_ROOT "C:/path/to/boost_1_82_0")  # Укажите путь к корневой директории Boost

# Include Boost directories
set(BOOST_INCLUDEDIR ${BOOST_ROOT}/include)
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib)

# Include Boost libraries
find_package(Boost 1.82 REQUIRED COMPONENTS filesystem json)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CHAT_LIBRARIES)

set(Qt6Modules Core Widgets)
foreach(QtModule IN LISTS Qt6Modules)
    list(APPEND CHAT_LIBRARIES Qt6::${QtModule})
endforeach(QtModule)

find_package(Qt6 COMPONENTS ${Qt6Modules} REQUIRED LinguistTools)

file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE HEADERS_FILES sources/*.h)
file(GLOB_RECURSE RC_FILES sources/*.qrc)
file(GLOB_RECURSE UI_FILES sources/*.ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt6_wrap_ui(QUI ${UI_FILES})
qt6_add_resources(QRC ${RC_FILES})

foreach(_ts_file ${TS_FILES})
    get_filename_component(_ts_name ${_ts_file} NAME_WE)
    add_custom_command(
        OUTPUT "${_ts_file}"
        DEPENDS ${TS_SOURCES}
        COMMAND ${Qt6_LUPDATE_EXECUTABLE} -recursive
        ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
    )
    add_custom_target(translate_${_ts_name} DEPENDS ${_ts_file})
    add_dependencies(translations translate_${_ts_name})
endforeach()

qt6_add_translation(QM_FILES ${TS_FILES})

add_executable(Chat ${QM_FILES} ${SRC_FILES} ${HEADERS_FILES}
    ${RC_FILES} ${UI_FILES}
    mainwindow.h mainwindow.cpp mainwindow.ui main.cpp
    startscreen.h startscreen.cpp startscreen.ui
    loginform.h loginform.cpp loginform.ui
    registrationform.h registrationform.cpp registrationform.ui
)

target_include_directories(Chat PUBLIC ${BOOST_INCLUDEDIR})
target_link_libraries(Chat PUBLIC ${CHAT_LIBRARIES} Boost::filesystem Boost::json)

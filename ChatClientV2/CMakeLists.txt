cmake_minimum_required(VERSION 3.12)

project(Chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опция CMAKE_INCLUDE_CURRENT_DIR обязательно должна быть включена,
# если вы не меняли пути генерации файлов для Qt.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Здесь мы создаем переменную CHAT_LIBRARIES, в которой будем хранить
# все подключаемые к проекту библиотеки, в т.ч. и библиотеки Qt
set(CHAT_LIBRARIES)

# Здесь мы создаем и заполняем список подключаемых модулей Qt
# Если нужно добавить модули, добавьте их в этот список по образцу:
# set(Qt5Modules [модуль 1] [модуль 2] ... [модуль n])
set(Qt6Modules Core Widgets)

# Здесь мы формируем из списка модулей список библиотек,
# которые требуется подключить, и складываем их в CHAT_LIBRARIES
foreach(QtModule IN LISTS Qt6Modules)
    list(APPEND CHAT_LIBRARIES Qt6::${QtModule})
endforeach(QtModule)

# Здесь проверяется наличие всех необходимых модулей и компонентов Qt
find_package(Qt6 COMPONENTS ${Qt6Modules} REQUIRED LinguistTools)

# Найти библиотеку Boost
find_package(Boost 1.82 REQUIRED COMPONENTS filesystem)

# Убедитесь, что Boost найден
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    list(APPEND CHAT_LIBRARIES ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find Boost")
endif()

# Здесь мы формируем список всех исходников, ассоциированных с проектом Chat
file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE HEADERS_FILES sources/*.h)
file(GLOB_RECURSE RC_FILES sources/*.qrc)
file(GLOB_RECURSE UI_FILES sources/*.ui)

# Включаем обязательные для нас опции
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Вызываем обработку .ui и .qrc файлов. Здесь же происходит генерация
# промежуточных исходников
qt6_wrap_ui(QUI ${UI_FILES})
qt6_add_resources(QRC ${RC_FILES})

# Здесь происходит создание файлов переводов и установка зависимостей
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

# Подключаем файлы трансляции (переводов)
qt6_add_translation(QM_FILES ${TS_FILES})

add_executable(Chat
    ${QM_FILES}
    ${SRC_FILES}
    ${HEADERS_FILES}
    ${RC_FILES}
    ${UI_FILES}
    mainwindow.h mainwindow.cpp mainwindow.ui
    main.cpp
    startscreen.h startscreen.cpp startscreen.ui
    loginform.h loginform.cpp loginform.ui
    registrationform.h registrationform.cpp registrationform.ui
)

# Подключаем к проекту библиотеки
target_link_libraries(Chat
    PUBLIC
    ${CHAT_LIBRARIES}
)

project(dtng)

cmake_minimum_required(VERSION 3.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(QT_MINIMUM_VERSION "5.6.3")

# Find the QtWidgets library
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Widgets)
find_package(Dtk REQUIRED Core Widget)
find_package(QTermWidget5 REQUIRED)

if(QTERMWIDGET_VERSION)
ADD_DEFINITIONS(-DOLD_QTERMWIDGET_VERSION)
endif()

# Populate a CMake variable with the sources
# TODO: portable headers?
file (GLOB DTNG_HEADER_FILES "*.h")
file (GLOB DTNG_CPP_FILES "*.cpp")

# Program
set(EXE_NAME dterm)

add_executable (${EXE_NAME}
    ${DTNG_HEADER_FILES}
    ${DTNG_CPP_FILES}
)

target_include_directories(${EXE_NAME} PUBLIC
                                        ${DtkWidget_INCLUDE_DIRS}
                                        ${PROJECT_BINARY_DIR}
                                        ${QTERMWIDGET_INCLUDE_DIRS}
)

target_link_libraries (dterm Qt5::Widgets ${DtkWidget_LIBRARIES} qtermwidget5)

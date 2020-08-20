set(PROJECT_NAME "DrawPixel")#设置项目名到PROJECT_NAME变量，后面用于指定项目名称和生成的文件名
project(${PROJECT_NAME})#设置项目名称
cmake_minimum_required(VERSION 2.6)#指定Cmake最低版本

#设置和平台相关的代码
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_DIR "../../../platform/Windows")#设置平台路径为windows
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_DIR "../../../platform/Linux")#设置平台路径为Linux
else()
	message(FATAL_ERROR "unknow operate system" ${CMAKE_SYSTEM_NAME})
endif ()

file(GLOB PLATFORM_SRC "${PLATFORM_DIR}/*.cpp")#将PLATFORM_DIR路径下的所有.cpp文件用完整路径风格赋值到PLATFORM_SRC变量
source_group(platform FILES ${PLATFORM_SRC})#创建一个名为platform的源码分组，并包含PLATFORM_DIR的的所有文件
aux_source_directory(. DIR_SRCS)#将当前目录(.)下的所有文件放入DIR_SRCS变量中
include_directories( "${PLATFORM_DIR}/include")#添加PLATFROM_DIR/include到头文件目录
add_executable(${PROJECT_NAME} ${DIR_SRCS} ${PLATFORM_SRC})#指定最后生成的可执行文件
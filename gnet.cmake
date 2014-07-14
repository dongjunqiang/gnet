include("${CMAKE_SOURCE_DIR}/common.cmake")

# 版本号
set(MAJOR_VERSION 0)
set(MINOR_VERSION 1)

# 目录
set(GNET_DIR_3RD "${CMAKE_SOURCE_DIR}/3rd")
set(GNET_DIR_PROTO "${CMAKE_SOURCE_DIR}/proto")
set(GNET_DIR_SRC "${CMAKE_SOURCE_DIR}/src")
set(GNET_DIR_GW "${CMAKE_SOURCE_DIR}/gw")

# 包含头文件
set(GNET_DIR_3RD_INCLUDE "${GNET_DIR_3RD}/include")
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${GNET_DIR_3RD_INCLUDE}
)

# 链接(gbase依赖curl和openssl, 需要时再加)
set(GNET_LIB_LINK "${COMMON_LINK_LIN}" protobuf)
link_directories("${GNET_DIR_3RD}/lib")

# 发布目录
string(TIMESTAMP TS "%y%m%d")
set(GNET_DIR_RELEASE "${CMAKE_SOURCE_DIR}/release-${MAJOR_VERSION}.${MINOR_VERSION}.${TS}")

# 编译选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

# 转换protocol
execute_process(
    COMMAND sh gen.sh
    WORKING_DIRECTORY ${GNET_DIR_PROTO}
)
message("gnet proto protoc convert complete.")

# 编译lib
aux_source_directory(${GNET_DIR_PROTO} GNET_LIB_SOURCE)
aux_source_directory(${GNET_DIR_SRC} GNET_LIB_SOURCE)
foreach(GNET_LIB_SOURCE_FILE ${GNET_LIB_SOURCE})
    CommonEcho(COLOR CYAN "===> lib source: ${GNET_LIB_SOURCE_FILE}")
endforeach()
add_library(gnet ${GNET_LIB_SOURCE})

# 安装到发布目录(todo)
install(
    DIRECTORY ${GNET_DIR_PROTO}
    DESTINATION "${GNET_DIR_RELEASE}"
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.conf"
)
install(
    TARGETS gnet
    DESTINATION "${GNET_DIR_RELEASE}/lib"
)


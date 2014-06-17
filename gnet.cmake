include("${CMAKE_SOURCE_DIR}/common.cmake")

# 版本号
set(MAJOR_VERSION 0)
set(MINOR_VERSION 1)

# 目录
set(GNET_DIR_3RD "${CMAKE_SOURCE_DIR}/3rd")
set(GNET_DIR_PROTO "${CMAKE_SOURCE_DIR}/proto")
set(GNET_DIR_CONF "${CMAKE_SOURCE_DIR}/conf")
set(GNET_DIR_SERVICE "${CMAKE_SOURCE_DIR}/service")

# 包含头文件
set(GNET_DIR_3RD_INCLUDE "${GNET_DIR_3RD}/include")
include_directories("${CMAKE_SOURCE_DIR}" ${GNET_DIR_3RD_INCLUDE})

# 链接(gbase依赖curl和openssl, 需要时再加)
set(GNET_DIR_3RD_LIB "${GNET_DIR_3RD}/lib")
set(GNET_LIB_LINK "${COMMON_LINK_LIN}" gbase protobuf)

# 发布目录
string(TIMESTAMP TS "%y%m%d")
set(GNET_DIR_RELEASE "${CMAKE_SOURCE_DIR}/release-${MAJOR_VERSION}.${MINOR_VERSION}.${TS}")

# 编译选项
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

# 转换protocol
execute_process(
    COMMAND sh gen.sh
    WORKING_DIRECTORY ${GNET_DIR_PROTO}
)
message("gnet proto protoc convert complete.")

# 转换conf
execute_process(
    COMMAND sh gen.sh
    WORKING_DIRECTORY ${GNET_DIR_CONF}
)
message("gnet conf protoc convert complete.")

# 编译lib的源文件
aux_source_directory(${GNET_DIR_PROTO} GNET_SOURCE)
aux_source_directory(${GNET_DIR_CONF} GNET_SOURCE)
aux_source_directory(${GNET_DIR_SERVICE} GNET_SOURCE)
foreach(GNET_SOURCE_FILE ${GNET_SOURCE})
    CommonEcho(COLOR CYAN "===> source: ${GNET_SOURCE_FILE}")
endforeach()

# 编译lib
add_library(gnet ${GNET_SOURCE})

# 安装到发布目录(todo)
install(
    DIRECTORY ${GNET_DIR_CONF}
    DESTINATION "${GNET_DIR_RELEASE}"
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.conf" 
)
install(
    TARGETS gnet 
    DESTINATION "${GNET_DIR_RELEASE}/lib"
)


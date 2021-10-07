find_path(LIB_RABBITMQ_C_ROOT_DIR
    NAMES include/rabbitmq-c/amqp.h
)

find_library(LIB_RABBITMQ_C_LIBRARIES
    NAMES rabbitmq
    HINTS ${LIB_RABBITMQ_C_ROOT_DIR}/build/librabbitmq
)

find_path(LIB_RABBITMQ_C_INCLUDE_DIR
    NAMES include/rabbitmq-c/amqp.h
    HINTS ${LIB_RABBITMQ_C_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIB_RABBITMQ_C DEFAULT_MSG
    LIB_RABBITMQ_C_LIBRARIES
    LIB_RABBITMQ_C_INCLUDE_DIR
)

mark_as_advanced(
    LIB_RABBITMQ_C_ROOT_DIR
    LIB_RABBITMQ_C_LIBRARIES
    LIB_RABBITMQ_C_INCLUDE_DIR
)

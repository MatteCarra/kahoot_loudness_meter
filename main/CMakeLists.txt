set(COMPONENT_SRCS "main.c" "sensor/sensor_thread.c" "sensor/sensor_thread.h" "tcp/tcp_protocol.c" "tcp/tcp_protocol.h" "data/circular_buffer.c" "tcp/socket_utils.c" "tcp/socket_utils.h")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

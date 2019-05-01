set(COMPONENT_SRCS "main.c" "sensor/sensor_thread.c" "sensor/sensor_thread.h" "udp/udp_protocol.c" "udp/udp_protocol.h" "data/circular_buffer.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

find_package(Protobuf 3.3.0 QUIET REQUIRED)

if (Protobuf_FOUND)
  set_target_properties(protobuf::libprotobuf
                        protobuf::protoc
                        PROPERTIES IMPORTED_GLOBAL TRUE)
else ()
  message(STATUS "Try to set PROTOBUF_ROOT_DIR, for example cmake -DPROTOBUF_ROOT_DIR=/usr/local")
endif ()
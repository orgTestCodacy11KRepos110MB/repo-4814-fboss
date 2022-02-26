# CMake to build libraries and binaries in fboss/agent

# In general, libraries and binaries in fboss/foo/bar are built by
# cmake/FooBar.cmake

add_library(fsdb_stream_client
  fboss/fsdb/client/FsdbStreamClient.cpp
  fboss/fsdb/client/oss/FsdbStreamClient.cpp
)

target_link_libraries(fsdb_stream_client
  Folly::folly
  FBThrift::thriftcpp2
)

# Part of Beast

GroupSources(extras/beast extras)
GroupSources(include/beast beast)
GroupSources(test/websocket "/")

add_executable (websocket-tests
    ${BEAST_INCLUDES}
    ${EXTRAS_INCLUDES}
    ../../extras/beast/unit_test/main.cpp
    websocket_async_echo_server.hpp
    websocket_sync_echo_server.hpp
    doc_snippets.cpp
    error.cpp
    option.cpp
    rfc6455.cpp
    stream.cpp
    teardown.cpp
    frame.cpp
    mask.cpp
    utf8_checker.cpp
)

target_link_libraries(websocket-tests Beast ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_COROUTINE_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_CONTEXT_LIBRARY})

if (MINGW)
    set_target_properties(websocket-tests PROPERTIES COMPILE_FLAGS "-Wa,-mbig-obj -Og")
endif()

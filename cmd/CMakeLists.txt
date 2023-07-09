
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

include_directories(../src)

# ax compiler executable
add_executable(axcomp axcomp.cc)

target_link_libraries(axcomp PUBLIC project_options project_warnings libax fmt ${llvm_libs} ${ICU_LIBRARIES})

install(TARGETS axcomp DESTINATION bin)
install(FILES ax DESTINATION bin PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

file (GLOB_RECURSE PROJ_CC_FILES 
  ${CMAKE_SOURCE_DIR}/src/*.hh
  ${CMAKE_SOURCE_DIR}/gui/*.hh
  ${CMAKE_SOURCE_DIR}/demos/*.hh
  ${CMAKE_SOURCE_DIR}/src/*.cc
  ${CMAKE_SOURCE_DIR}/gui/*.cc
  ${CMAKE_SOURCE_DIR}/demos/*.cc
)


add_custom_target (format "clang-format" -i ${PROJ_CC_FILES} --style=file  COMMENT "Formatting source code...")

foreach (tidy_target ${PROJ_CC_FILES})
  get_filename_component (basename ${tidy_target} NAME)
  get_filename_component (dirname ${tidy_target} DIRECTORY)
  get_filename_component (basedir ${dirname} NAME)
  set (tidy_target_name "${basedir}__${basename}")
  set (tidy_command clang-tidy --quiet --format-style=file -p=${PROJECT_BINARY_DIR} ${tidy_target})
  add_custom_target (tidy_${tidy_target_name} ${tidy_command})
  list (APPEND PROJ_TIDY_TARGETS tidy_${tidy_target_name})
endforeach (tidy_target)

add_custom_target (tidy DEPENDS ${PROJ_TIDY_TARGETS})

option(WERROR "Set warnings as errors" OFF)

function(set_warnings_as_errors target)
  if (WERROR)
    target_compile_options(
      ${target} PRIVATE
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -Wno-gnu-zero-variadic-macro-arguments>
      $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
      )
  endif()
endfunction()

CMAKE_MINIMUM_REQUIRED ( VERSION 3.1 )

PROJECT ( Cutepiler )

# Remove strings matching given regular expression from a list.
# @param(in,out) aItems Reference of a list variable to filter.
# @param aRegEx Value of regular expression to match.
function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

FILE ( GLOB_RECURSE src_list "src/*.cpp" "test/*.cpp" )
filter_items(src_list "src/asm_generator/*.cpp")

SET ( prog_name cutepiler )

SET ( CMAKE_INSTALL_PREFIX )

SET ( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
ADD_EXECUTABLE ( ${prog_name} ${src_list} )

target_link_libraries( ${prog_name} pthread )

SET(CMAKE_CXX_COMPILER  "clang++")
SET(CMAKE_CXX_FLAGS "-Wall -O2 -std=c++17 -Wno-logical-op-parentheses ${CMAKE_CXX_FLAGS}")

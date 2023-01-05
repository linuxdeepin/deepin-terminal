#=============================================================================
# Copyright 2019 Gary Wang
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Gary Wang nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

function(DTK_CREATE_I18N_FROM_JSON _generated_file_list _input_json_file _output_cpp_file_name)
    set (DTK_SETTINGS_TOOLS_EXECUTABLE ${DTKCORE_TOOL_DIR}/dtk-settings)
    set (generated_file_list) # 0(failed) or 1(successed) files in the list.

    get_filename_component(_input_json_abs_path ${_input_json_file} ABSOLUTE)
    get_filename_component(_input_json_abs_dir ${_input_json_abs_path} DIRECTORY)
    set (_output_cpp_abs_path ${_input_json_abs_dir}/${_output_cpp_file_name})
    
    if (EXISTS ${DTK_SETTINGS_TOOLS_EXECUTABLE})
        add_custom_command(OUTPUT ${_output_cpp_abs_path}
            COMMAND ${DTK_SETTINGS_TOOLS_EXECUTABLE}
            ARGS ${_input_json_abs_path} -o ${_output_cpp_abs_path}
            DEPENDS ${_input_json_abs_path} VERBATIM)
        list(APPEND generated_file_list ${_output_cpp_abs_path})
    else ()
        message ("dtk-settings-tools could not be found at ${DTK_SETTINGS_TOOLS_EXECUTABLE}")
    endif ()

    set(${_generated_file_list} ${generated_file_list} PARENT_SCOPE)
endfunction()

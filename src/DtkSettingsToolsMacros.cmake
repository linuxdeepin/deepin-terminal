# Copyright 2019 Gary Wang
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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

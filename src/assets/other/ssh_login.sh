#! /usr/bin/expect -f

# Copyright (C) 2011 ~ 2016 Deepin, Inc.
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

## All possible interactive messages:
# Are you sure you want to continue connecting (yes/no)?
# password:
# Enter passphrase for key

## Main
# Delete self for secret, will not affect the following code
file delete $argv0

# Setup variables
# Set timeout -1 to avoid remote server dis-connect.
set timeout -1
set user [lindex $argv 0]
set server [lindex $argv 1]
set port [lindex $argv 2]
set private_key [lindex $argv 3]
set password [lindex $argv 4]
set authentication {<<AUTHENTICATION>>}
set ssh_cmd {zssh -X -o ServerAliveInterval=60}
set ssh_opt {$user@$server -p $port -o PubkeyAuthentication=$authentication}
set remote_command {<<REMOTE_COMMAND>>}

# fix bug#64758 修改服务器密码，点击连接没有密码错误提示语，且可以成功连接
# 将ascii值转换为字符串，并保存到encoded_password变量中。
set encoded_password ""
set hexstring $password
set len [string length $hexstring]
for {set i 0} {$i < [expr $len / 2]} {incr i} {
    set j [expr 2 * $i]
    set char [string range $hexstring $j [expr $j + 1]]
    set encoded_password $encoded_password[format %c [format %i 0x$char]]
}

# This code is use for synchronous pty's size to avoid terminal not update if login in remote server.
trap {
    stty rows [stty rows] columns [stty columns] < $spawn_out(slave,name)
} WINCH

# Spawn and expect
if { $authentication == "no" } {
eval spawn $ssh_cmd $ssh_opt -t $remote_command exec \\\$SHELL -l
} else {
eval spawn $ssh_cmd $ssh_opt -i $private_key -t $remote_command exec \\\$SHELL -l
}

if { [string length $password] } {
    expect {
        timeout {send_user "ssh connection time out, please operate manually\n"}
        -nocase "yes/no" {send "yes\r"; exp_continue}
        -nocase -re "password:|enter passphrase for key" {
            send "$encoded_password\r"
        }
    }
}
interact


#!/bin/sh
count=`ps -elf|awk '{if($15=="deepin-terminal")print $15}'|wc -l`
return $count

#!/usr/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#检查有没有deepin-terminal终端被安装
dt=`which deepin-terminal`
echo $dt
if [ ! $dt ] ; then
    echo "没有发现 deepin-terminal 终端被安装" 
    exit 0
fi


version=`deepin-terminal -v`

version=$(echo $version | sed -e 's/\(.*\)deepin-terminal\(.*\)/\2/g' -e 's/ //g')

if [ ! "$version" ]; then
#  version=5.2.19
  echo "deepin-terminal 没有获取到相应的版本号"
  exit 0
fi

version=$(echo $version | sed -e 's/\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/g')

echo $version
#固定的头查询
#fixhead="parse commandLine is ok" 
fixhead="new terminal start run" 
#固定的尾查询
fixtail="m_enableShareMemory set true 1" 
#创建终端延时时间
creat_term_delay=0.5
#cpu采样次数
cpu_resample_number=10

#用于5.2.19版本
fixid="app create all complete, MainWindowID = "


#打开log日志
strdate=$(date +"%Y-%m-%d-%H-%M-%S")
accfile=$(pwd)/acc_$strdate.txt
#echo $logfile

if [ -f $accfile ]; then
    rm -rf $accfile
fi

#检检查是不是使用deepin-terminal终端
pid=$(ps -eo pid,ppid,sid,comm | grep 'deepin-terminal' | awk '{print $1;}')

if [ $pid ]; then
    echo "为了准确性,关闭deepin-terminal 终端,使用其他终端测试" 
    exit 0
fi

dtl=~/.cache/deepin/deepin-terminal/deepin-terminal.log
if [ -f $dtl ] ; then
    rm -rf $dtl
    sleep 0.2
fi



#默认100个终端
count=10

#如果输入终端，则使用输入的终端
if [ $1 ] ; then
    count=$1
fi


echo "理论启动终端个数:$count"

#每隔100ms启动终端

#for (( i = 0; i < count; i++ )); do
#    deepin-terminal &
#    sleep 0.1
#done
i=0
firstvmhw=0
firstvmsz=0
first=1
while [ $i -lt $count ] 
do
    deepin-terminal &
    sleep $creat_term_delay
    i=$(grep -c "create NormalWindow, current count" $dtl)
   # if [ $i == 1 ]; then
   #     pid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print $1;}')
#        firstvmhw=$(grep "VmHWM" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
#        firstvmsz=$(grep "VmSize" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
#        first=0
#   fi
    if [ $first == 1 ]; then
        pid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print $1;}')
        firstvmhw=$(grep "VmHWM" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
        firstvmsz=$(grep "VmSize" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
        first=0
    fi
done

pid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print $1}')

if [ ! $pid ]; then
    echo "没有终端被启动"
    exit 0
fi

echo "分析中,请稍候..."
sleep 5

pid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print $1}')
#echo "TermPid:  $pid" >> $accfile
#启动终端数
#echo "启动终端总数:  $tn" >> $accfile
echo "终端版本:  $version" >> $accfile
echo "CPU平台:  $(uname -a|cut -d " " -f 3)" >> $accfile
#启止时间
starttime=$(head -n +1 $dtl)
starttime=${starttime%%[*}
starttime=${starttime/, / }
echo "开始时间:  $starttime" >> $accfile

tn=$(grep -c "create NormalWindow, current count" $dtl)
endtime=$(grep "create NormalWindow, current count = $tn" $dtl)
endtime=${endtime%%[*}
endtime=${endtime/, / }
echo "结束时间:  ${endtime}" >> $accfile

startstmp=$(date -d "$starttime" +%s%N)
endstmp=$(date -d "$endtime" +%s%N)
tottime=$[endstmp-startstmp]
tottime=$[tottime/1000000]

tottime=$(echo "scale=2;$tottime/1000"|bc)
echo "总计用时(S):  $tottime" >> $accfile


pid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print $1;}')

#cat "/proc/$pid/status" | grep "Name" >> $accfile
#cat "/proc/$pid/status" | grep "VmPeak" >> $accfile
#cat "/proc/$pid/status" | grep "VmSize" >> $accfile
#cat "/proc/$pid/status" | grep "VmHWM" >> $accfile
#cat "/proc/$pid/status" | grep "VmPTE" >> $accfile
#$(grep "Name" /proc/$pid/status | cut -d ":" -f 2)

#vmhwm=$(grep "VmHWM" /proc/$pid/status | cut -d ":" -f 2)


firstvmhw=$(echo $firstvmhw|sed -e 's/^[ \t]*//g')
#firstvmhw=$[firstvmhw/1000]
firstvmhw=$(echo "scale=3; $firstvmhw/1000"|bc)

echo "启动物理内存(M):  $firstvmhw" >> $accfile

firstvmsz=$(echo $firstvmsz|sed -e 's/^[ \t]*//g')
#firstvmsz=$[firstvmsz/1000]
firstvmsz=$(echo "scale=3; $firstvmsz/1000"|bc)
echo "启动虚拟内存(M):  $firstvmsz" >> $accfile



vmhwm=$(grep "VmHWM" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
vmhwm=$(echo $vmhwm|sed -e 's/^[ \t]*//g')
#vmhwm=$[vmhwm/1000]
vmhwm=$(echo "scale=3; $vmhwm/1000"|bc)
echo "最终物理内存(M):  $vmhwm" >> $accfile

vmsize=$(grep "VmSize" /proc/$pid/status | cut -d ":" -f 2 | cut -d "k" -f 1)
vmsize=$(echo $vmsize|sed -e 's/^[ \t]*//g')
#vmsize=$[vmsize/1000]
vmsize=$(echo "scale=3; $vmsize/1000"|bc)
echo "最终虚拟内存(M):  $vmsize" >> $accfile


#cat "/proc/$pid/status" | grep "voluntary_ctxt_switches" >> $accfile
#cat "/proc/$pid/status" | grep "nonvoluntary_ctxt_switches" >> $accfile

#cpu平均使用率
avgcpu=0;
i=0
echo "cpu采样次数为: $cpu_resample_number"
termpid=$(ps -eo pid,comm | grep 'deepin-terminal' | awk '{print$1;}')
for userpid in $termpid; do
	if [ $userpid ] ; then
	 	break
	fi
done
while [ $i -lt $cpu_resample_number ]
do
 #   curcpu=$(top -n1 | grep "deepin-terminal" | awk '{print $10;}')
#    curcpu=$(ps -eo pcpu,comm | grep "deepin-terminal" | awk '{print $1;}')
    curcpu=`top -n 1 -d 1 -p $userpid | grep deepin | awk '{print $10;}'`
#    echo "====================$curcpu=================="
     if [ $curcpu ] ; then
        # avgcpu=$((avgcpu+curcpu))
        avgcpu=$(echo "scale=2; $avgcpu+$curcpu"|bc)
	i=$((i+1))
	echo "cpu采样第$i次"
    fi
    sleep 1
done
#avgcpu=$((avgcpu/3))
avgcpu=$(echo "scale=2; $avgcpu/$cpu_resample_number"|bc)
echo "最终平均CPU占用:   $avgcpu %" >> $accfile



tcount=$(grep -c "create NormalWindow, current count" $dtl)
#echo "$tcount" >> $accfile
tottime=0
mintime=999999999999
maxtime=0
firsttime=0
lasttime=0
cmpversion=$(echo "$version" | sed 's/\.//g')

if [ $cmpversion -ge 5219 ] ; then
    for(( i =1; i <= tcount; i++ )); do
        orgStr=$(grep "$fixid $i ," $dtl)
        echo $orgStr
        tmptime=$(echo $orgStr | sed 's/\(.*\)all time use \(.*\) ms/\2/g') 
        tottime=$[tottime+tmptime]
        
        if [ $i -eq 1 ]; then
            firsttime=$tmptime
        fi

        if [ $i -eq $tcount ] ; then
            lasttime=$tmptime
        fi

        if [ $maxtime -lt $tmptime ]; then
            maxtime=$tmptime
        fi

        if [ $tmptime -lt $mintime ]; then
            mintime=$tmptime
        fi
    done
    tottime=$[tottime*1000000]
    mintime=$[mintime*1000000]
    maxtime=$[maxtime*1000000]
    firsttime=$[firsttime*1000000]
    lasttime=$[lasttime*1000000]
else

    for (( ii = 1; ii <= tcount; ii++ )); do
        orgStr="create NormalWindow, current count = $ii , SingleFlag"
        line=$(grep -n "$orgStr" $dtl | cut -d ":" -f 1)
        for(( ik=line; ik > 0 ; ik-- )); do
            #   str=$(head -n +$ik $dtl)
            str=$(sed -n $ik\p $dtl)
            #if [[ $str =~ "parse commandLine is ok" ]] 
            if [[ $str =~ $fixhead ]]; then
                #    tendtime=$(grep "create NormalWindow, current count = $ii , SingleFlag" $dtl)
                tstarttime=${str%%[*}
                tstarttime=${tstarttime/, / }
                tstarttime=$(date -d "$tstarttime" +%s%N)

          #  tendtime=${tendtime%%[*}
          #  tendtime=${tendtime/, / }
          #  tendtime=$(date -d "$tendtime" +%s%N)
          #     echo "WhichTerm:  $ii" >> $accfile
          #     echo "  StartTime:  $tstarttime" >> $accfile
          #     echo "  EndTime:    $tendtime" >> $accfile
          #  tmptime=$[tendtime-tstarttime]
          # tottime=$[tottime+tmptime]
          break
            fi
        done

        for(( ik=line; ; ik++ )); do                                                                      
            str=$(sed -n $ik\p $dtl)                                                                         
            if [[ $str =~ $fixtail ]]                                                
            then                                                                                             
                tendtime=${str%%[*}                                                                          
                tendtime=${tendtime/, / }                                                                    
                tendtime=$(date -d "$tendtime" +%s%N)
                break                                                                                        
            fi                                                                                              
        done                                                                                                

        echo "终端创建开始时间:  $tstarttime"                                                                    
        echo "终端创建结束时间:  $tendtime"  

        if [ $tstarttime -a $tendtime ] ; then                                                           
            tmptime=$[tendtime-tstarttime]                             
            tottime=$[tottime+tmptime]
            
            #首次启动时间
            if [ $firsttime -eq 0 ]; then
                firsttime=$tmptime 
            fi
                
            if [ $ii -eq $tcount ]; then
                lasttime=$tmptime
            fi


            #最大启动时间
            if [ $maxtime -lt $tmptime ]; then
                maxtime=$tmptime
            fi
            #最小启动时间
            if [ $tmptime -lt $mintime ]; then
                mintime=$tmptime
            fi
            fi 
        done
fi

echo "首个终端启动时间:  $[firsttime/1000000] 毫秒" >> $accfile
echo "末尾终端启动时间:  $[lasttime/1000000] 毫秒" >> $accfile
echo "终端最小启动时间:  $[mintime/1000000] 毫秒" >> $accfile
echo "终端最大启动时间:  $[maxtime/1000000] 毫秒" >> $accfile
tottime=$[tottime/1000000]
avgtime=$[avgtime/1000000]
echo "所有程序真实启动时间:  $tottime 毫秒" >> $accfile
avgtime=$[tottime/tcount]
echo "所有程序真实平均时间: $avgtime 毫秒" >> $accfile


echo "分析完成,请在$accfile 文件中查看"

#echo $(grep "" $accfile)
cat $accfile


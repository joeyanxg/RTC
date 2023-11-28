#!/bin/sh

echo "modufy route table,ffeng add "
busybox ip rule add from all lookup main pref 9000

echo "get rtc time"
hwclock -s

# midea Path
mideaPath='/data/midea/opt/'
# main process path
mainProcessPath=${mideaPath}'seven/'
# ota File path
otaFilesPath=${mideaPath}'ota-files/'
# ota tmp path
otaTmpPath=${otaFilesPath}'tmp/'
#system path
systemPath='/system/midea/'
# ota File Name
otaFileName='tempapp.tar.gz'
# ota script Name
otaScriptName='ug.sh'
# boot script Name
bootScriptName='mideaBoot.sh'

echo ${mainProcessPath}

# 检测 ota目录 /data/midea/opt/ota-files/ 是否存在，没有则创建
if [ ! -d "${otaFilesPath}" ];then
  mkdir -p ${otaFilesPath} 2>/dev/null
fi

# 检测 ota目录中的临时目录是否存在 /data/midea/opt/ota-files/tmp 是否存在，没有则创建
if [ ! -d "${otaTmpPath}" ];then
  mkdir -p ${otaTmpPath} 2>/dev/null
fi

# 检测是否存在 /data/midea/opt/mideaBoot.sh 不存在说明系统未初始化，将 系统自带更新包 /system/midea/tempapp.tar.gz 复制到data中的OTA目录中 /data/midea/opt/ota-files/ 目录
if [ ! -e "${mideaPath}${bootScriptName}" ];then
  if [ -e "${systemPath}${otaFileName}" ];then
    cp -rf ${systemPath}${otaFileName} ${otaFilesPath}
  fi
fi

# 检测文件 /data/midea/opt/ota-files/tempapp.tar.gz 是否存在. 存在则加ota文件进行解压到临时目录中 /data/midea/opt/ota-files/tmp/
if [ -e "${otaFilesPath}${otaFileName}" ];then
  busybox tar -zxf ${otaFilesPath}${otaFileName} -C ${otaTmpPath} 2>/dev/null
  if [ "x$?" == "x0" ];then
    # 检查 updateFiles 文件夹 以及 升级脚本 ug.sh 是否存在，存在则为 ug.sh 赋予执行权限，并运行脚本
    if [ -d "${otaTmpPath}/updateFiles" ];then
      if [ -e "${otaTmpPath}/updateFiles/px30/ug.sh" ];then
        cd ${otaTmpPath}/updateFiles/px30
        chmod +x ug.sh
				./ug.sh >/dev/null 2>&1
        sync
      fi
    fi
  fi
  # 清空 临时目录及升级文件
  rm -rf ${otaTmpPath} 2>/dev/null
  rm -f ${otaFilesPath}${otaFileName} 2>/dev/null
fi

# 检测 启动脚本是否存在. 若存在则赋予权限并执行
if [ -e "${mideaPath}${bootScriptName}" ];then
  cd ${mideaPath}
  chmod +x ${bootScriptName}
  ./${bootScriptName}
fi
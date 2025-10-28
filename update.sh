#!/bin/bash

# 配置 - 请在每个设备上修改这些变量
TARGET_DIR="/Users/desktop/Desktop/code"  # 修改为你的目录
METADATA_FILE=".sync_metadata"
LOG_FILE="$HOME/scripts/git_smart_sync.log"
BRANCH="main"
DEVICE_ID="mac"  # 在每个设备上设置不同的ID

# 检测操作系统并设置兼容的命令
detect_os() {
    case "$(uname -s)" in
        Darwin)
            OS="macos"
            STAT_MTIME_CMD="stat -f %m"
            ;;
        Linux)
            OS="linux"
            STAT_MTIME_CMD="stat -c %Y"
            ;;
        *)
            OS="unknown"
            STAT_MTIME_CMD="stat -c %Y"  # 默认使用Linux格式
            ;;
    esac
    log "检测到操作系统: $OS"
}

# 记录日志
log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$LOG_FILE"
}

# 生成文件哈希（跨平台兼容）
get_file_hash() {
    if [ -f "$1" ]; then
        if command -v shasum >/dev/null 2>&1; then
            # macOS
            shasum -a 256 "$1" | cut -d' ' -f1
        elif command -v sha256sum >/dev/null 2>&1; then
            # Linux
            sha256sum "$1" | cut -d' ' -f1
        else
            # 备用方案
            openssl sha256 "$1" 2>/dev/null | cut -d' ' -f2
        fi
    else
        echo "deleted"
    fi
}

# 获取文件修改时间（跨平台兼容）
get_file_mtime() {
    if [ -f "$1" ]; then
        eval "$STAT_MTIME_CMD \"$1\""
    else
        echo "0"
    fi
}

# 生成元数据
generate_metadata() {
    echo "# 自动生成的同步元数据" > "$METADATA_FILE"
    echo "# 设备: $DEVICE_ID" >> "$METADATA_FILE"
    echo "# 操作系统: $OS" >> "$METADATA_FILE"
    echo "# 生成时间: $(date -Iseconds)" >> "$METADATA_FILE"
    echo "" >> "$METADATA_FILE"
    
    # 使用find查找所有文件，排除.git目录和元数据文件本身
    find . -type f -not -path "./.git/*" -not -name "$METADATA_FILE" | while read file; do
        if [ -f "$file" ]; then
            # 移除开头的 ./
            file="${file#./}"
            hash=$(get_file_hash "$file")
            mtime=$(get_file_mtime "$file")
            echo "$file|$hash|$mtime|$DEVICE_ID|$OS" >> "$METADATA_FILE"
        fi
    done
}

# 解析元数据文件
parse_metadata() {
    local metadata_file="$1"
    declare -gA file_hash file_mtime file_device file_os
    
    while IFS='|' read -r file hash mtime device os; do
        # 跳过注释和空行
        [[ "$file" =~ ^# ]] && continue
        [[ -z "$file" ]] && continue
        
        file_hash["$file"]="$hash"
        file_mtime["$file"]="$mtime"
        file_device["$file"]="$device"
        file_os["$file"]="$os"
    done < "$metadata_file"
}

# 比较文件并决定操作
compare_and_sync() {
    local local_meta="$1"
    local remote_meta="$2"
    
    declare -A local_hash local_mtime local_device local_os
    declare -A remote_hash remote_mtime remote_device remote_os
    
    # 解析本地元数据
    parse_metadata "$local_meta"
    for key in "${!file_hash[@]}"; do
        local_hash["$key"]="${file_hash[$key]}"
        local_mtime["$key"]="${file_mtime[$key]}"
        local_device["$key"]="${file_device[$key]}"
        local_os["$key"]="${file_os[$key]}"
    done
    
    # 解析远程元数据
    parse_metadata "$remote_meta"
    for key in "${!file_hash[@]}"; do
        remote_hash["$key"]="${file_hash[$key]}"
        remote_mtime["$key"]="${file_mtime[$key]}"
        remote_device["$key"]="${file_device[$key]}"
        remote_os["$key"]="${file_os[$key]}"
    done
    
    # 比较文件
    all_files=($(printf "%s\n" "${!local_hash[@]}" "${!remote_hash[@]}" | sort -u))
    
    local need_sync=false
    local conflicts=()
    
    for file in "${all_files[@]}"; do
        local_local_hash="${local_hash[$file]}"
        local_remote_hash="${remote_hash[$file]}"
        local_local_mtime="${local_mtime[$file]:-0}"
        local_remote_mtime="${remote_mtime[$file]:-0}"
        
        # 文件只在本地存在
        if [ -z "$local_remote_hash" ] && [ "$local_local_hash" != "deleted" ]; then
            log "新文件: $file (仅在本地设备: ${local_device[$file]})"
            continue
        fi
        
        # 文件只在远程存在
        if [ -z "$local_local_hash" ] && [ "$local_remote_hash" != "deleted" ]; then
            log "新文件: $file (仅在远程设备: ${remote_device[$file]})"
            # 从远程恢复
            if git show "origin/$BRANCH:$file" > "$file" 2>/dev/null; then
                log "恢复文件: $file"
                need_sync=true
            fi
            continue
        fi
        
        # 文件在两边都存在但不同
        if [ "$local_local_hash" != "$local_remote_hash" ]; then
            # 根据修改时间决定
            if [ "$local_local_mtime" -gt "$local_remote_mtime" ]; then
                log "使用本地版本: $file (本地较新 - ${local_device[$file]})"
                # 本地版本更新，保留本地
            elif [ "$local_remote_mtime" -gt "$local_local_mtime" ]; then
                log "使用远程版本: $file (远程较新 - ${remote_device[$file]})"
                # 远程版本更新，使用远程
                if git show "origin/$BRANCH:$file" > "$file" 2>/dev/null; then
                    log "更新文件: $file"
                    need_sync=true
                fi
            else
                # 时间相同但内容不同，记录冲突
                log "冲突: $file (修改时间相同但内容不同)"
                conflicts+=("$file")
            fi
        fi
    done
    
    # 处理冲突文件
    if [ ${#conflicts[@]} -gt 0 ]; then
        log "发现 ${#conflicts[@]} 个冲突文件"
        for conflict in "${conflicts[@]}"; do
            # 默认使用本地版本，但重命名远程版本
            if [ -f "$conflict" ]; then
                cp "$conflict" "$conflict.conflict.${remote_device[$conflict]}"
                log "保留本地版本，远程版本保存为: $conflict.conflict.${remote_device[$conflict]}"
            fi
        done
        need_sync=true
    fi
    
    echo "$need_sync"
}

# 检查Git状态
check_git_status() {
    cd "$TARGET_DIR" || return 1
    
    # 检查是否在Git仓库中
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        log "错误: 目录不是Git仓库"
        return 1
    fi
    
    # 检查远程仓库配置
    if ! git remote get-url origin > /dev/null 2>&1; then
        log "错误: 未配置远程仓库origin"
        return 1
    fi
    
    return 0
}

# 主同步函数
smart_sync() {
    cd "$TARGET_DIR" || {
        log "错误: 无法进入目录 $TARGET_DIR"
        return 1
    }
    
    # 检测操作系统
    detect_os
    
    # 配置用户信息
    if [ -z "$(git config user.name)" ]; then
        git config user.name "Auto Sync - $DEVICE_ID"
        git config user.email "roy20130108@126.com"
    fi
    
    # 检查Git状态
    if ! check_git_status; then
        log "Git状态检查失败"
        return 1
    fi
    
    # 清理状态
    git rebase --abort 2>/dev/null || true
    git merge --abort 2>/dev/null || true
    rm -fr ".git/rebase-merge" 2>/dev/null || true
    rm -fr ".git/rebase-apply" 2>/dev/null || true
    
    # 获取远程更新
    log "获取远程更新..."
    if ! git fetch origin; then
        log "错误: 获取远程更新失败"
        return 1
    fi
    
    # 生成当前元数据
    log "生成本地元数据..."
    generate_metadata
    cp "$METADATA_FILE" "/tmp/local_meta.$$"
    
    # 获取远程元数据
    if git show "origin/$BRANCH:$METADATA_FILE" > "/tmp/remote_meta.$$" 2>/dev/null; then
        log "找到远程元数据，开始智能比较..."
        need_sync=$(compare_and_sync "/tmp/local_meta.$$" "/tmp/remote_meta.$$")
    else
        log "未找到远程元数据，首次同步"
        need_sync=true
    fi
    
    # 检查是否有需要提交的更改
    if [ -n "$(git status --porcelain)" ] || [ "$need_sync" = true ]; then
        log "检测到需要同步的更改..."
        
        # 重新生成元数据（可能文件有变化）
        generate_metadata
        
        # 添加并提交
        git add .
        git commit -m "智能同步: $(date '+%Y-%m-%d %H:%M:%S') - 设备: $DEVICE_ID ($OS)"
        
        # 推送
        if git push origin "$BRANCH"; then
            log "同步成功"
        else
            log "推送失败，尝试拉取后重推..."
            if git pull --rebase origin "$BRANCH"; then
                git push origin "$BRANCH" && log "拉取后推送成功" || log "推送仍然失败"
            else
                log "拉取失败，需要手动解决冲突"
            fi
        fi
    else
        log "没有需要同步的更改"
    fi
    
    # 清理临时文件
    rm -f "/tmp/local_meta.$$" "/tmp/remote_meta.$$"
}

# 主程序
log "开始智能同步 - 设备: $DEVICE_ID"
if smart_sync; then
    log "智能同步完成"
else
    log "智能同步失败"
fi

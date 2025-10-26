#!/bin/bash

# 配置
TARGET_DIR="/Users/desktop/Desktop/code"  # 请修改为你的实际目录
LOG_FILE="$HOME/scripts/git_auto_commit.log"

# 记录日志
log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$LOG_FILE"
}

# 进入目录
cd "$TARGET_DIR" || {
    log "错误: 无法进入目录 $TARGET_DIR"
    exit 1
}

# 确保用户信息已配置
if [ -z "$(git config user.name)" ]; then
    git config user.name "Auto Committer"
    git config user.email "roy20130108@126.com"
fi

# 清理任何可能的冲突状态
git rebase --abort 2>/dev/null || true
git merge --abort 2>/dev/null || true
rm -fr ".git/rebase-merge" 2>/dev/null || true
rm -fr ".git/rebase-apply" 2>/dev/null || true

# 检查是否有文件变化
if [ -n "$(git status --porcelain)" ]; then
    log "检测到文件变化，执行提交..."
    
    # 添加并提交所有变化
    git add .
    git commit -m "自动提交: $(date '+%Y-%m-%d %H:%M:%S')"
    
    # 强制推送（适用于备份场景）
    if git push --force-with-lease origin main 2>/dev/null; then
        log "提交并强制推送成功"
    else
        log "强制推送失败，可能需要手动解决"
    fi
else
    log "没有检测到文件变化"
    
    # 即使没有变化，也确保分支同步
    LOCAL_COMMIT=$(git log --oneline -1 --format=%H)
    REMOTE_COMMIT=$(git ls-remote origin main | cut -f1)
    
    if [ "$LOCAL_COMMIT" != "$REMOTE_COMMIT" ]; then
        log "检测到分支偏离，强制同步..."
        git push --force-with-lease origin main 2>/dev/null && \
        log "分支同步成功" || \
        log "分支同步失败"
    fi
fi

import os
import shutil
import logging
import time
from pathlib import Path
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class FileClassifierHandler(FileSystemEventHandler):
    """文件系统事件处理器，用于监控文件变化并自动分类"""
    
    def __init__(self, source_dir, destination_dirs, file_extensions):
        self.source_dir = source_dir
        self.destination_dirs = destination_dirs
        self.file_extensions = file_extensions
        self.setup_logging()
        
        # 创建目标目录
        self.create_destination_dirs()
        
        # 记录最近处理过的文件，避免重复处理
        self.recently_processed = {}
        
    def setup_logging(self):
        """设置日志记录"""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
    
    def create_destination_dirs(self):
        """创建目标目录（如果不存在）"""
        for dir_path in self.destination_dirs.values():
            Path(dir_path).mkdir(parents=True, exist_ok=True)
            logging.info(f"确保目录存在: {dir_path}")
    
    def classify_file(self, file_path):
        """分类单个文件"""
        # 检查是否为文件
        if not os.path.isfile(file_path):
            return False
            
        filename = os.path.basename(file_path)
        _, ext = os.path.splitext(filename)
        ext = ext.lower()
        
        # 防止重复处理：检查文件是否最近被处理过
        current_time = time.time()
        if file_path in self.recently_processed:
            if current_time - self.recently_processed[file_path] < 2:  # 2秒内不重复处理
                return False
        
        # 检查扩展名是否在映射中
        if ext in self.file_extensions:
            target_dir = self.destination_dirs[self.file_extensions[ext]]
            
            try:
                # 复制文件到目标目录
                shutil.copy2(file_path, target_dir)
                logging.info(f"已自动分类: {filename} -> {target_dir}")
                
                # 记录处理时间
                self.recently_processed[file_path] = current_time
                return True
                
            except Exception as e:
                logging.error(f"分类文件 {filename} 时出错: {str(e)}")
                return False
        else:
            logging.debug(f"跳过文件 {filename} (扩展名 {ext} 未在分类规则中)")
            return False
    
    def on_modified(self, event):
        """文件修改时触发"""
        if not event.is_directory:
            self.classify_file(event.src_path)
    
    def on_created(self, event):
        """文件创建时触发"""
        if not event.is_directory:
            # 等待一小段时间，确保文件完全写入
            time.sleep(0.5)
            self.classify_file(event.src_path)
    
    def on_moved(self, event):
        """文件移动时触发"""
        if not event.is_directory:
            self.classify_file(event.dest_path)

def initial_classification(source_dir, file_extensions, destination_dirs):
    """初始分类：处理已经存在的文件"""
    logging.info("开始初始文件分类...")
    processed_count = 0
    
    for filename in os.listdir(source_dir):
        file_path = os.path.join(source_dir, filename)
        if os.path.isfile(file_path):
            handler = FileClassifierHandler(source_dir, destination_dirs, file_extensions)
            if handler.classify_file(file_path):
                processed_count += 1
    
    logging.info(f"初始分类完成，处理了 {processed_count} 个文件")

def start_monitoring(source_dir, destination_dirs, file_extensions):
    """开始监控目录"""
    logging.info(f"开始监控目录: {source_dir}")
    
    # 创建事件处理器
    event_handler = FileClassifierHandler(source_dir, destination_dirs, file_extensions)
    
    # 创建观察者
    observer = Observer()
    observer.schedule(event_handler, source_dir, recursive=False)
    
    # 启动观察者
    observer.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
        logging.info("监控已停止")
    
    observer.join()

def main():
    """主函数"""
    # 定义目录路径
    source_dir = "/Users/desktop/Desktop/code/luogu"
    
    # 定义目标目录
    destination_dirs = {
        "php": "/Users/desktop/Desktop/code/php",
        "python": "/Users/desktop/Desktop/code/python", 
        "cpp": "/Users/desktop/Desktop/code/c++",
        "c": "/Users/desktop/Desktop/code/c"
    }
    
    # 定义文件扩展名到类别的映射
    file_extensions = {
        ".php": "php",
        ".py": "python", 
        ".cpp": "cpp",
        ".c": "c"
    }
    
    # 验证源目录是否存在
    if not os.path.exists(source_dir):
        logging.error(f"源目录不存在: {source_dir}")
        return
    
    if not os.path.isdir(source_dir):
        logging.error(f"源路径不是目录: {source_dir}")
        return
    
    print("文件自动分类监控器")
    print("=" * 50)
    print(f"监控目录: {source_dir}")
    print("分类规则:")
    for ext, category in file_extensions.items():
        print(f"  {ext} -> {destination_dirs[category]}")
    print("=" * 50)
    print("按 Ctrl+C 停止监控")
    
    # 先进行初始分类
    initial_classification(source_dir, file_extensions, destination_dirs)
    
    # 开始监控
    start_monitoring(source_dir, destination_dirs, file_extensions)

if __name__ == "__main__":
    main()

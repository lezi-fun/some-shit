import os
import shutil
import logging
from pathlib import Path

def setup_logging():
    """设置日志记录"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

def create_destination_dirs(destination_dirs):
    """创建目标目录（如果不存在）"""
    for dir_path in destination_dirs.values():
        Path(dir_path).mkdir(parents=True, exist_ok=True)
        logging.info(f"确保目录存在: {dir_path}")

def classify_files(source_dir, file_extensions, destination_dirs):
    """分类文件"""
    processed_files = 0
    errors = []
    
    # 遍历源目录中的所有文件
    for filename in os.listdir(source_dir):
        file_path = os.path.join(source_dir, filename)
        
        # 跳过目录，只处理文件
        if not os.path.isfile(file_path):
            continue
            
        # 获取文件扩展名
        _, ext = os.path.splitext(filename)
        ext = ext.lower()  # 转换为小写以确保匹配
        
        # 检查扩展名是否在映射中
        if ext in file_extensions:
            target_dir = destination_dirs[file_extensions[ext]]
            
            try:
                # 复制文件到目标目录
                shutil.copy2(file_path, target_dir)
                logging.info(f"已复制: {filename} -> {target_dir}")
                processed_files += 1
                
            except Exception as e:
                error_msg = f"复制文件 {filename} 时出错: {str(e)}"
                logging.error(error_msg)
                errors.append(error_msg)
        else:
            logging.debug(f"跳过文件 {filename} (扩展名 {ext} 未在分类规则中)")
    
    return processed_files, errors

def main():
    """主函数"""
    # 设置日志
    setup_logging()
    
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
    
    logging.info(f"开始处理源目录: {source_dir}")
    
    try:
        # 创建目标目录
        create_destination_dirs(destination_dirs)
        
        # 分类文件
        processed_files, errors = classify_files(source_dir, file_extensions, destination_dirs)
        
        # 输出结果摘要
        print("\n" + "="*50)
        print("文件分类完成!")
        print(f"处理的文件数量: {processed_files}")
        print(f"错误数量: {len(errors)}")
        
        if errors:
            print("\n发生的错误:")
            for error in errors:
                print(f"  - {error}")
        else:
            print("所有文件处理成功!")
            
    except Exception as e:
        logging.error(f"程序执行过程中发生错误: {str(e)}")

if __name__ == "__main__":
    main()

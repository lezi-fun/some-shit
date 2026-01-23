#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <regex>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

// 配置结构体
struct CompilerConfig {
    std::string cpp_standard = "c++23";
    std::vector<std::string> compile_options = {"-O2", "-Wall", "-Wextra"};
    std::vector<std::string> link_options = {};
    bool show_help_flag = false;
    bool run_after_compile = false;
    bool temp_mode = false;
    bool select_std = false;
    bool select_options = false;
};

class CppCompiler {
private:
    std::string filename;
    std::string output_name;
    CompilerConfig config;
    
    // 配置文件路径
    std::string config_file_path;
    
public:
    CppCompiler() {
        // 设置配置文件路径
        #ifdef _WIN32
        char* appdata = std::getenv("APPDATA");
        if (appdata) {
            config_file_path = std::string(appdata) + "\\cpp_compiler_config.txt";
        } else {
            config_file_path = "cpp_compiler_config.txt";
        }
        #else
        char* home = std::getenv("HOME");
        if (home) {
            config_file_path = std::string(home) + "/.cpp_compiler_config";
        } else {
            config_file_path = ".cpp_compiler_config";
        }
        #endif
    }
    
    void show_help() {
        std::cout << "用法: c+++ <文件名> [选项]\n"
                  << "\n"
                  << "选项:\n"
                  << "  -r, --run       编译后直接运行程序\n"
                  << "  -t, --temp      运行后删除编译的文件(需与-r一起使用)\n"
                  << "  -s, --set       设置并保存默认配置\n"
                  << "  -h, --help      显示此帮助信息\n"
                  << "  --show-config   显示当前配置\n"
                  << "\n"
                  << "示例:\n"
                  << "  c+++ main.cpp                # 使用保存的配置编译main.cpp\n"
                  << "  c+++ main.cpp -r             # 编译并运行main.cpp\n"
                  << "  c+++ main.cpp -rt            # 编译、运行并删除可执行文件\n"
                  << "  c+++ -s                       # 设置默认配置\n"
                  << "  c+++ --show-config           # 显示当前配置\n"
                  << "  c+++ main.cpp -r -t          # 编译、运行并删除\n"
                  << "\n"
                  << "注意：默认使用保存的配置，使用 -s 设置配置\n";
    }
    
    // 保存配置到文件
    bool save_config() {
        std::ofstream config_file(config_file_path);
        if (!config_file.is_open()) {
            std::cerr << "无法打开配置文件: " << config_file_path << "\n";
            return false;
        }
        
        // 写入配置
        config_file << "cpp_standard=" << config.cpp_standard << "\n";
        
        config_file << "compile_options=";
        for (size_t i = 0; i < config.compile_options.size(); ++i) {
            config_file << config.compile_options[i];
            if (i < config.compile_options.size() - 1) {
                config_file << ",";
            }
        }
        config_file << "\n";
        
        config_file << "link_options=";
        for (size_t i = 0; i < config.link_options.size(); ++i) {
            config_file << config.link_options[i];
            if (i < config.link_options.size() - 1) {
                config_file << ",";
            }
        }
        config_file << "\n";
        
        config_file.close();
        std::cout << "配置已保存到: " << config_file_path << "\n";
        return true;
    }
    
    // 从文件加载配置
    bool load_config() {
        std::ifstream config_file(config_file_path);
        if (!config_file.is_open()) {
            std::cout << "未找到配置文件，使用默认配置\n";
            return false;
        }
        
        std::string line;
        while (std::getline(config_file, line)) {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos) {
                std::string key = line.substr(0, equals_pos);
                std::string value = line.substr(equals_pos + 1);
                
                if (key == "cpp_standard") {
                    config.cpp_standard = value;
                } else if (key == "compile_options") {
                    config.compile_options.clear();
                    std::istringstream iss(value);
                    std::string option;
                    while (std::getline(iss, option, ',')) {
                        if (!option.empty()) {
                            config.compile_options.push_back(option);
                        }
                    }
                } else if (key == "link_options") {
                    config.link_options.clear();
                    std::istringstream iss(value);
                    std::string option;
                    while (std::getline(iss, option, ',')) {
                        if (!option.empty()) {
                            config.link_options.push_back(option);
                        }
                    }
                }
            }
        }
        
        config_file.close();
        std::cout << "已从配置文件加载配置\n";
        return true;
    }
    
    // 显示当前配置
    void show_current_config() {
        std::cout << "\n当前配置:\n";
        std::cout << "========================================\n";
        std::cout << "C++标准: " << config.cpp_standard << "\n";
        
        std::cout << "编译选项: ";
        for (const auto& opt : config.compile_options) {
            std::cout << opt << " ";
        }
        std::cout << "\n";
        
        std::cout << "链接选项: ";
        for (const auto& opt : config.link_options) {
            std::cout << opt << " ";
        }
        std::cout << "\n";
        
        std::cout << "配置文件路径: " << config_file_path << "\n";
        std::cout << "========================================\n";
    }
    
    // 显示配置菜单
    void show_config_menu() {
        std::cout << "\nC++编译器配置菜单\n";
        std::cout << "========================================\n";
        std::cout << "1. 选择C++标准版本\n";
        std::cout << "2. 选择编译和链接选项\n";
        std::cout << "3. 查看当前配置\n";
        std::cout << "4. 保存配置并退出\n";
        std::cout << "5. 退出不保存\n";
        std::cout << "========================================\n";
        std::cout << "请选择 (1-5): ";
    }
    
    // 选择C++标准
    void select_cpp_standard() {
        std::cout << "\n支持的C++标准版本:\n";
        std::cout << "  1) c++98    - 传统C++标准\n";
        std::cout << "  2) c++11    - C++11标准(2011年)\n";
        std::cout << "  3) c++14    - C++14标准(2014年)\n";
        std::cout << "  4) c++17    - C++17标准(2017年)\n";
        std::cout << "  5) c++20    - C++20标准(2020年)\n";
        std::cout << "  6) c++23    - C++23标准(2023年)\n";
        std::cout << "  7) c++2b    - 下一个C++标准\n";
        std::cout << "  8) gnu++98  - GNU C++98\n";
        std::cout << "  9) gnu++11  - GNU C++11\n";
        std::cout << "  10) gnu++14 - GNU C++14\n";
        std::cout << "  11) gnu++17 - GNU C++17\n";
        std::cout << "  12) gnu++20 - GNU C++20\n";
        std::cout << "  13) gnu++23 - GNU C++23\n";
        std::cout << "\n当前标准: " << config.cpp_standard << "\n";
        std::cout << "请选择标准 (1-13) 或输入标准名称: ";
        
        std::string input;
        std::getline(std::cin, input);
        
        std::map<std::string, std::string> standard_map = {
            {"1", "c++98"}, {"2", "c++11"}, {"3", "c++14"}, {"4", "c++17"},
            {"5", "c++20"}, {"6", "c++23"}, {"7", "c++2b"},
            {"8", "gnu++98"}, {"9", "gnu++11"}, {"10", "gnu++14"},
            {"11", "gnu++17"}, {"12", "gnu++20"}, {"13", "gnu++23"}
        };
        
        if (standard_map.find(input) != standard_map.end()) {
            config.cpp_standard = standard_map[input];
        } else {
            // 验证输入的标准是否有效
            std::vector<std::string> valid_standards = {
                "c++98", "c++11", "c++14", "c++17", "c++20", "c++23", "c++2b",
                "gnu++98", "gnu++11", "gnu++14", "gnu++17", "gnu++20", "gnu++23", "gnu++2b"
            };
            
            if (std::find(valid_standards.begin(), valid_standards.end(), input) != valid_standards.end()) {
                config.cpp_standard = input;
            } else {
                std::cout << "无效的标准，保持为: " << config.cpp_standard << "\n";
            }
        }
        
        std::cout << "C++标准已设置为: " << config.cpp_standard << "\n";
    }
    
    // 选择编译和链接选项
    void select_compile_options() {
        std::cout << "\n编译和链接选项配置:\n";
        std::cout << "========================================\n";
        std::cout << "当前编译选项: ";
        for (const auto& opt : config.compile_options) {
            std::cout << opt << " ";
        }
        std::cout << "\n";
        
        std::cout << "当前链接选项: ";
        for (const auto& opt : config.link_options) {
            std::cout << opt << " ";
        }
        std::cout << "\n\n";
        
        std::cout << "1. 添加优化选项\n";
        std::cout << "2. 添加调试选项\n";
        std::cout << "3. 添加警告选项\n";
        std::cout << "4. 添加链接选项\n";
        std::cout << "5. 清空所有选项\n";
        std::cout << "6. 返回上一级\n";
        std::cout << "========================================\n";
        std::cout << "请选择 (1-6): ";
        
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            std::cout << "\n优化选项:\n";
            std::cout << "  a) -O0 - 无优化(调试)\n";
            std::cout << "  b) -O1 - 基础优化\n";
            std::cout << "  c) -O2 - 推荐优化(默认)\n";
            std::cout << "  d) -O3 - 完全优化\n";
            std::cout << "  e) -Os - 优化代码大小\n";
            std::cout << "  f) -Ofast - 激进优化\n";
            std::cout << "请选择 (a-f): ";
            
            std::string opt_choice;
            std::getline(std::cin, opt_choice);
            
            // 移除现有的优化选项
            std::vector<std::string> optimization_opts = {"-O0", "-O1", "-O2", "-O3", "-Os", "-Ofast"};
            for (const auto& opt : optimization_opts) {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), opt);
                if (it != config.compile_options.end()) {
                    config.compile_options.erase(it);
                    break; // 一次只移除一个优化选项
                }
            }
            
            // 添加新的优化选项
            if (opt_choice == "a") config.compile_options.push_back("-O0");
            else if (opt_choice == "b") config.compile_options.push_back("-O1");
            else if (opt_choice == "c") config.compile_options.push_back("-O2");
            else if (opt_choice == "d") config.compile_options.push_back("-O3");
            else if (opt_choice == "e") config.compile_options.push_back("-Os");
            else if (opt_choice == "f") config.compile_options.push_back("-Ofast");
            else std::cout << "无效选择，保持当前设置\n";
            
        } else if (choice == "2") {
            std::cout << "\n调试选项:\n";
            std::cout << "  a) -g - 生成调试信息\n";
            std::cout << "  b) -g3 - 生成完整调试信息\n";
            std::cout << "  c) -ggdb - 生成GDB调试信息\n";
            std::cout << "  d) 删除调试选项\n";
            std::cout << "请选择 (a-d): ";
            
            std::string opt_choice;
            std::getline(std::cin, opt_choice);
            
            if (opt_choice == "a") {
                // 移除现有的调试选项
                std::vector<std::string> debug_opts = {"-g", "-g3", "-ggdb"};
                for (const auto& opt : debug_opts) {
                    auto it = std::find(config.compile_options.begin(), config.compile_options.end(), opt);
                    if (it != config.compile_options.end()) {
                        config.compile_options.erase(it);
                    }
                }
                config.compile_options.push_back("-g");
            } else if (opt_choice == "b") {
                std::vector<std::string> debug_opts = {"-g", "-g3", "-ggdb"};
                for (const auto& opt : debug_opts) {
                    auto it = std::find(config.compile_options.begin(), config.compile_options.end(), opt);
                    if (it != config.compile_options.end()) {
                        config.compile_options.erase(it);
                    }
                }
                config.compile_options.push_back("-g3");
            } else if (opt_choice == "c") {
                std::vector<std::string> debug_opts = {"-g", "-g3", "-ggdb"};
                for (const auto& opt : debug_opts) {
                    auto it = std::find(config.compile_options.begin(), config.compile_options.end(), opt);
                    if (it != config.compile_options.end()) {
                        config.compile_options.erase(it);
                    }
                }
                config.compile_options.push_back("-ggdb");
            } else if (opt_choice == "d") {
                std::vector<std::string> debug_opts = {"-g", "-g3", "-ggdb"};
                for (const auto& opt : debug_opts) {
                    auto it = std::find(config.compile_options.begin(), config.compile_options.end(), opt);
                    if (it != config.compile_options.end()) {
                        config.compile_options.erase(it);
                    }
                }
            }
            
        } else if (choice == "3") {
            std::cout << "\n警告选项:\n";
            std::cout << "  a) -Wall - 启用所有警告\n";
            std::cout << "  b) -Wextra - 启用额外警告\n";
            std::cout << "  c) -Werror - 将警告视为错误\n";
            std::cout << "  d) -Wpedantic - 严格ISO C++警告\n";
            std::cout << "  e) 切换选项状态\n";
            std::cout << "请选择 (a-e): ";
            
            std::string opt_choice;
            std::getline(std::cin, opt_choice);
            
            if (opt_choice == "a") {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), "-Wall");
                if (it == config.compile_options.end()) {
                    config.compile_options.push_back("-Wall");
                } else {
                    config.compile_options.erase(it);
                }
            } else if (opt_choice == "b") {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), "-Wextra");
                if (it == config.compile_options.end()) {
                    config.compile_options.push_back("-Wextra");
                } else {
                    config.compile_options.erase(it);
                }
            } else if (opt_choice == "c") {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), "-Werror");
                if (it == config.compile_options.end()) {
                    config.compile_options.push_back("-Werror");
                } else {
                    config.compile_options.erase(it);
                }
            } else if (opt_choice == "d") {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), "-Wpedantic");
                if (it == config.compile_options.end()) {
                    config.compile_options.push_back("-Wpedantic");
                } else {
                    config.compile_options.erase(it);
                }
            }
            
        } else if (choice == "4") {
            std::cout << "\n链接选项:\n";
            std::cout << "  a) -static - 静态链接\n";
            std::cout << "  b) -static-libstdc++ - 静态链接C++标准库\n";
            std::cout << "  c) -shared - 生成共享库\n";
            std::cout << "  d) -fPIC - 位置无关代码\n";
            std::cout << "  e) 删除链接选项\n";
            std::cout << "请选择 (a-e): ";
            
            std::string opt_choice;
            std::getline(std::cin, opt_choice);
            
            if (opt_choice == "a") {
                auto it = std::find(config.link_options.begin(), config.link_options.end(), "-static");
                if (it == config.link_options.end()) {
                    config.link_options.push_back("-static");
                } else {
                    config.link_options.erase(it);
                }
            } else if (opt_choice == "b") {
                auto it = std::find(config.link_options.begin(), config.link_options.end(), "-static-libstdc++");
                if (it == config.link_options.end()) {
                    config.link_options.push_back("-static-libstdc++");
                } else {
                    config.link_options.erase(it);
                }
            } else if (opt_choice == "c") {
                auto it = std::find(config.link_options.begin(), config.link_options.end(), "-shared");
                if (it == config.link_options.end()) {
                    config.link_options.push_back("-shared");
                } else {
                    config.link_options.erase(it);
                }
            } else if (opt_choice == "d") {
                auto it = std::find(config.compile_options.begin(), config.compile_options.end(), "-fPIC");
                if (it == config.compile_options.end()) {
                    config.compile_options.push_back("-fPIC");
                } else {
                    config.compile_options.erase(it);
                }
            } else if (opt_choice == "e") {
                config.link_options.clear();
            }
            
        } else if (choice == "5") {
            config.compile_options.clear();
            config.link_options.clear();
            // 恢复默认优化选项
            config.compile_options.push_back("-O2");
            std::cout << "已清空所有选项，恢复默认设置\n";
        }
    }
    
    // 配置模式
    void config_mode() {
        std::cout << "C++编译器配置模式\n";
        load_config(); // 加载现有配置
        
        while (true) {
            show_config_menu();
            
            std::string choice;
            std::getline(std::cin, choice);
            
            if (choice == "1") {
                select_cpp_standard();
            } else if (choice == "2") {
                select_compile_options();
            } else if (choice == "3") {
                show_current_config();
            } else if (choice == "4") {
                save_config();
                std::cout << "配置已保存，退出配置模式\n";
                break;
            } else if (choice == "5") {
                std::cout << "退出配置模式，未保存更改\n";
                break;
            } else {
                std::cout << "无效选择，请重试\n";
            }
        }
    }
    
    bool check_gcc() {
        #ifdef _WIN32
        if (system("g++ --version > nul 2>&1") != 0) {
        #else
        if (system("g++ --version > /dev/null 2>&1") != 0) {
        #endif
            std::cerr << "错误: 未找到g++编译器\n"
                      << "请安装g++: \n"
                      << "  Ubuntu/Debian: sudo apt install g++\n"
                      << "  macOS: brew install gcc\n"
                      << "  Windows: 安装MinGW或MSYS2\n";
            return false;
        }
        
        return true;
    }
    
    bool parse_arguments(int argc, char* argv[]) {
        if (argc == 1) {
            show_help();
            return false;
        }
        
        // 先检查是否是特殊命令
        std::string first_arg = argv[1];
        if (first_arg == "-s" || first_arg == "--set") {
            config_mode();
            return false; // 配置模式后不需要继续执行
        }
        
        if (first_arg == "--show-config") {
            load_config();
            show_current_config();
            return false;
        }
        
        if (first_arg == "-h" || first_arg == "--help") {
            config.show_help_flag = true;
            return true;
        }
        
        // 加载配置文件
        load_config();
        
        // 解析其他参数
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg.length() > 2 && arg[0] == '-' && arg[1] != '-') {
                for (size_t j = 1; j < arg.length(); j++) {
                    char opt = arg[j];
                    if (opt == 'h') {
                        config.show_help_flag = true;
                        return true;
                    } else if (opt == 'r') {
                        config.run_after_compile = true;
                    } else if (opt == 't') {
                        config.temp_mode = true;
                    } else {
                        std::cerr << "错误: 未知选项 -" << opt << "\n";
                        show_help();
                        return false;
                    }
                }
            } else if (arg == "-h" || arg == "--help") {
                config.show_help_flag = true;
                return true;
            } else if (arg == "-r" || arg == "--run") {
                config.run_after_compile = true;
            } else if (arg == "-t" || arg == "--temp") {
                config.temp_mode = true;
            } else if (arg[0] == '-') {
                std::cerr << "错误: 未知选项 " << arg << "\n";
                show_help();
                return false;
            } else if (filename.empty()) {
                filename = arg;
            } else {
                std::cerr << "错误: 只能指定一个源文件\n";
                show_help();
                return false;
            }
        }
        
        if (filename.empty()) {
            std::cerr << "错误: 请指定源文件\n";
            show_help();
            return false;
        }
        
        if (config.temp_mode && !config.run_after_compile) {
            std::cerr << "错误: -t(临时模式)必须与-r(运行)选项一起使用\n";
            show_help();
            return false;
        }
        
        return true;
    }
    
    bool validate_file() {
        if (!fs::exists(filename)) {
            std::cerr << "错误: 文件 '" << filename << "' 不存在\n";
            return false;
        }
        
        std::vector<std::string> valid_extensions = {".cpp", ".cxx", ".cc", ".c++", ".C", ".cp"};
        fs::path file_path(filename);
        std::string ext = file_path.extension().string();
        
        bool has_valid_extension = false;
        for (const auto& valid_ext : valid_extensions) {
            if (ext == valid_ext) {
                has_valid_extension = true;
                break;
            }
        }
        
        if (!has_valid_extension) {
            std::cout << "警告: 文件 '" << filename << "' 可能不是C++源文件\n"
                      << "继续编译吗？(y/N): ";
            std::string confirm;
            std::getline(std::cin, confirm);
            if (confirm.empty() || (confirm[0] != 'y' && confirm[0] != 'Y')) {
                return false;
            }
        }
        
        output_name = file_path.stem().string();
        
        #ifdef _WIN32
        output_name += ".exe";
        #endif
        
        return true;
    }
    
    bool compile() {
        if (!check_gcc()) {
            return false;
        }
        
        std::cout << "正在编译 " << filename << " ...\n";
        std::cout << "使用配置: C++标准=" << config.cpp_standard << "\n";
        
        // 构建编译命令
        std::stringstream cmd;
        cmd << "g++ -std=" << config.cpp_standard;
        
        for (const auto& opt : config.compile_options) {
            cmd << " " << opt;
        }
        
        for (const auto& opt : config.link_options) {
            cmd << " " << opt;
        }
        
        cmd << " \"" << filename << "\" -o \"" << output_name << "\"";
        
        std::string full_cmd = cmd.str();
        std::cout << "执行: " << full_cmd << "\n";
        
        int result = system(full_cmd.c_str());
        
        return (result == 0);
    }
    
    bool run_program() {
        if (!config.run_after_compile) {
            return true;
        }
        
        std::cout << "运行 " << output_name << ":\n";
        std::cout << "----------------------------------------\n";
        
        #ifdef _WIN32
        std::string run_cmd = "\"" + output_name + "\"";
        #else
        std::string run_cmd = "./\"" + output_name + "\"";
        #endif
        
        int result = system(run_cmd.c_str());
        
        std::cout << "----------------------------------------\n";
        std::cout << "程序退出代码: " << result << "\n";
        
        if (config.temp_mode) {
            std::cout << "删除编译的文件: " << output_name << "\n";
            std::remove(output_name.c_str());
        }
        
        return true;
    }
    
    int execute(int argc, char* argv[]) {
        if (!parse_arguments(argc, argv)) {
            return config.show_help_flag ? 0 : 1;
        }
        
        if (config.show_help_flag) {
            show_help();
            return 0;
        }
        
        if (!validate_file()) {
            return 1;
        }
        
        if (!compile()) {
            std::cerr << "✗ 编译失败\n";
            return 1;
        }
        
        std::cout << "✓ 编译成功: " << output_name << "\n";
        
        if (!run_program()) {
            return 1;
        }
        
        return 0;
    }
};

int main(int argc, char* argv[]) {
    CppCompiler compiler;
    return compiler.execute(argc, argv);
}

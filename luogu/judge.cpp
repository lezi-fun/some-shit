#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <regex>
#include <map>
#include <libgen.h>
#include <limits.h>

using namespace std;

// 评测结果枚举
enum JudgeResult {
    AC = 0,  // 通过
    WA,      // 答案错误
    TLE,     // 超时
    MLE,     // 内存超限
    RE,      // 运行时错误
    UKE,     // 未知错误
    CE       // 编译错误
};

// 配置结构体
struct Config {
    int total_score = 100;          // 总分
    bool enable_subtask = false;    // 是否开启子任务
    bool special_judge = false;     // 是否为Special Judge
    bool interactive = false;       // 是否为交互题
    bool communication = false;     // 是否为通信题
    int time_limit = 1000;          // 时间限制(ms)
    int memory_limit = 512;         // 内存限制(MB)
    vector<int> point_ratio;        // 每个测试点的分数比例
    vector<int> subtask_groups;     // 子任务分组
};

// 测试点信息
struct TestPoint {
    string input_file;
    string output_file;
    int point_ratio;
    JudgeResult result;
    double time_used;
    long memory_used;
};

// 工具函数：分割字符串
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// 获取可执行文件所在目录
string get_executable_dir() {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        char* dir = dirname(exe_path);
        return string(dir);
    }
    return ".";
}

// 读取配置文件
Config read_config(const string &config_file) {
    Config config;
    ifstream file(config_file);
    string line;
    
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        
        if (key == "总分") {
            config.total_score = stoi(value);
        } else if (key == "是否开启子任务") {
            config.enable_subtask = (value == "1" || value == "true");
        } else if (key == "是否为special_judge") {
            config.special_judge = (value == "1" || value == "true");
        } else if (key == "是否为交互题") {
            config.interactive = (value == "1" || value == "true");
        } else if (key == "是否为通信题") {
            config.communication = (value == "1" || value == "true");
        } else if (key == "任务点所占分数比例") {
            vector<string> ratios = split(value, ',');
            for (const auto &ratio : ratios) {
                config.point_ratio.push_back(stoi(ratio));
            }
        } else if (key == "子任务分组" && config.enable_subtask) {
            vector<string> groups = split(value, ',');
            for (const auto &group : groups) {
                config.subtask_groups.push_back(stoi(group));
            }
        } else if (key == "时间限制(ms)") {
            config.time_limit = stoi(value);
        } else if (key == "内存限制(MB)") {
            config.memory_limit = stoi(value);
        }
    }
    
    file.close();
    return config;
}

// 从文件名中提取数字
int extract_number_from_filename(const string &filename) {
    // 使用正则表达式匹配文件名中的数字
    regex pattern("\\d+");
    smatch match;
    
    if (regex_search(filename, match, pattern)) {
        return stoi(match.str());
    }
    
    return -1;  // 没有找到数字
}

// 获取测试点列表
vector<TestPoint> get_test_points(const string &task_dir, const vector<int> &ratios) {
    vector<TestPoint> test_points;
    map<int, pair<string, string>> file_map;  // 使用map按数字排序
    
    // 读取目录中的所有文件
    DIR *dir = opendir(task_dir.c_str());
    if (dir == nullptr) {
        cerr << "无法打开目录: " << task_dir << endl;
        return test_points;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;
        
        // 跳过当前目录和上级目录
        if (filename == "." || filename == "..") {
            continue;
        }
        
        // 检查文件扩展名并提取数字
        string full_path = task_dir + "/" + filename;
        int num = extract_number_from_filename(filename);
        
        if (num == -1) {
            continue;  // 文件名中没有数字，跳过
        }
        
        // 检查文件类型
        if (filename.find(".in") != string::npos) {
            // 是输入文件
            ifstream file(full_path);
            if (file.is_open()) {
                file.close();
                file_map[num].first = full_path;
            }
        } else if (filename.find(".out") != string::npos) {
            // 是输出文件
            ifstream file(full_path);
            if (file.is_open()) {
                file.close();
                file_map[num].second = full_path;
            }
        }
    }
    
    closedir(dir);
    
    // 创建测试点列表（按数字排序）
    int index = 0;
    for (const auto &entry : file_map) {
        int num = entry.first;
        const auto &files = entry.second;
        
        // 检查是否同时有输入和输出文件
        if (!files.first.empty() && !files.second.empty()) {
            TestPoint point;
            point.input_file = files.first;
            point.output_file = files.second;
            point.point_ratio = (index < ratios.size()) ? ratios[index] : 1;
            point.result = UKE;
            point.time_used = 0;
            point.memory_used = 0;
            
            test_points.push_back(point);
            index++;
        } else {
            cerr << "警告: 测试点" << num << "缺少输入或输出文件" << endl;
        }
    }
    
    // 按数字排序
    sort(test_points.begin(), test_points.end(),
         [](const TestPoint &a, const TestPoint &b) {
             int num_a = extract_number_from_filename(a.input_file);
             int num_b = extract_number_from_filename(b.input_file);
             return num_a < num_b;
         });
    
    return test_points;
}

// 编译C++代码
bool compile_cpp(const string &source_file, const string &executable, bool use_testlib = false) {
    string exe_dir = get_executable_dir();
    string command;
    
    if (use_testlib) {
        // 包含testlib.h路径
        command = "g++ -std=c++11 -O2 -I" + exe_dir + " -o " + executable + " " + source_file + " 2> /tmp/compile_error.txt";
    } else {
        command = "g++ -std=c++11 -O2 -o " + executable + " " + source_file + " 2> /tmp/compile_error.txt";
    }
    
    int ret = system(command.c_str());
    
    if (ret != 0) {
        cout << "编译错误: " << source_file << endl;
        ifstream error_file("/tmp/compile_error.txt");
        if (error_file.is_open()) {
            string line;
            while (getline(error_file, line)) {
                cout << line << endl;
            }
            error_file.close();
        }
        return false;
    }
    return true;
}

// 运行程序并收集资源使用情况
JudgeResult run_program(const string &program, const string &input_file, 
                       const string &output_file, int time_limit, 
                       int memory_limit, double &time_used, long &memory_used) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // 子进程
        // 设置资源限制
        rlimit rl;
        rl.rlim_cur = (time_limit / 1000.0) + 1;  // 秒
        rl.rlim_max = rl.rlim_cur;
        setrlimit(RLIMIT_CPU, &rl);
        
        rl.rlim_cur = memory_limit * 1024 * 1024;  // 转换为字节
        rl.rlim_max = rl.rlim_cur;
        setrlimit(RLIMIT_AS, &rl);
        
        // 重定向输入输出
        freopen(input_file.c_str(), "r", stdin);
        freopen(output_file.c_str(), "w", stdout);
        freopen("/tmp/program_stderr.txt", "w", stderr);
        
        execl(program.c_str(), program.c_str(), NULL);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // 父进程
        int status;
        struct rusage usage;
        wait4(pid, &status, 0, &usage);
        
        // 获取时间和内存使用
        time_used = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0 +
                   (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000.0;
        memory_used = usage.ru_maxrss;  // KB
        
        // 检查结果
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                // 检查时间和内存限制
                if (time_used > time_limit) {
                    return TLE;
                }
                if (memory_used > memory_limit * 1024) {  // 转换为KB
                    return MLE;
                }
                return AC;
            } else {
                return RE;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGXCPU || sig == SIGALRM) {
                return TLE;
            } else if (sig == SIGSEGV || sig == SIGABRT) {
                return RE;
            }
        }
        return UKE;
    }
    return UKE;
}

// 普通评测：比较输出文件
JudgeResult normal_judge(const string &std_output, const string &user_output) {
    ifstream std_file(std_output);
    ifstream user_file(user_output);
    
    if (!std_file.is_open() || !user_file.is_open()) {
        return UKE;
    }
    
    string std_line, user_line;
    while (getline(std_file, std_line)) {
        if (!getline(user_file, user_line)) {
            return WA;
        }
        
        // 去除行尾空格
        std_line.erase(std_line.find_last_not_of(" \t\n\r\f\v") + 1);
        user_line.erase(user_line.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (std_line != user_line) {
            return WA;
        }
    }
    
    // 检查用户输出是否有多余行
    if (getline(user_file, user_line)) {
        user_line.erase(user_line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (!user_line.empty()) {
            return WA;
        }
    }
    
    return AC;
}

// Special Judge评测 (使用testlib.h的checker)
JudgeResult special_judge(const string &spj_program, const string &input_file,
                         const string &std_output, const string &user_output) {
    // testlib格式的checker通常接受三个参数：输入文件、用户输出、标准输出
    // 或者四个参数：输入文件、用户输出、标准输出、结果文件
    // 我们使用三个参数的格式
    string command = spj_program + " " + input_file + " " + user_output + " " + std_output + " 2> /tmp/spj_error.txt";
    
    int ret = system(command.c_str());
    
    if (WIFEXITED(ret)) {
        int exit_code = WEXITSTATUS(ret);
        if (exit_code == 0) {
            return AC;
        } else if (exit_code == 1 || exit_code == 2) {
            return WA;
        } else {
            // 读取可能的错误信息
            ifstream error_file("/tmp/spj_error.txt");
            if (error_file.is_open()) {
                string line;
                while (getline(error_file, line)) {
                    cerr << "SPJ错误: " << line << endl;
                }
                error_file.close();
            }
            return UKE;
        }
    }
    
    return UKE;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "用法: " << argv[0] << " student.cpp task_folder" << endl;
        cerr << "示例: " << argv[0] << " solution.cpp ./testdata" << endl;
        return 1;
    }
    
    string student_cpp = argv[1];
    string task_dir = argv[2];
    
    // 读取配置文件
    string config_file = task_dir + "/env";
    Config config = read_config(config_file);
    
    // 编译学生代码
    if (!compile_cpp(student_cpp, "/tmp/student")) {
        cout << "学生代码编译失败" << endl;
        cout << "总分: 0" << endl;
        return 0;
    }
    
    // 如果需要，编译Special Judge代码 (使用checker.cpp和testlib.h)
    if (config.special_judge) {
        string checker_cpp = task_dir + "/checker.cpp";
        if (!compile_cpp(checker_cpp, "/tmp/checker", true)) {
            cerr << "Special Judge代码 (checker.cpp) 编译失败" << endl;
            return 1;
        }
    }
    
    // 获取测试点
    vector<TestPoint> test_points = get_test_points(task_dir, config.point_ratio);
    
    if (test_points.empty()) {
        cerr << "未找到测试点" << endl;
        cerr << "请确保测试文件夹中包含格式为 *.in 和 *.out 的文件，且文件名中包含数字（如 game001.in, game001.out）" << endl;
        return 1;
    }
    
    // 运行所有测试点
    double total_score = 0;
    int total_ratio = 0;
    for (auto &ratio : config.point_ratio) {
        total_ratio += ratio;
    }
    if (total_ratio == 0) total_ratio = test_points.size();
    
    cout << "开始评测..." << endl;
    cout << "测试点数量: " << test_points.size() << endl;
    cout << "时间限制: " << config.time_limit << "ms" << endl;
    cout << "内存限制: " << config.memory_limit << "MB" << endl;
    if (config.special_judge) {
        cout << "评测方式: Special Judge (使用testlib.h)" << endl;
    } else {
        cout << "评测方式: 文本比对" << endl;
    }
    cout << endl;
    
    for (size_t i = 0; i < test_points.size(); i++) {
        TestPoint &point = test_points[i];
        
        // 从文件名中提取测试点编号
        int point_num = extract_number_from_filename(point.input_file);
        string point_name = (point_num != -1) ? to_string(point_num) : to_string(i + 1);
        
        // 运行学生程序
        string student_output = "/tmp/student_out_" + to_string(i + 1) + ".txt";
        point.result = run_program("/tmp/student", point.input_file, 
                                 student_output, config.time_limit,
                                 config.memory_limit, point.time_used, point.memory_used);
        
        // 如果运行成功，进行评测
        if (point.result == AC) {
            if (config.special_judge) {
                point.result = special_judge("/tmp/checker", point.input_file,
                                           point.output_file, student_output);
            } else {
                point.result = normal_judge(point.output_file, student_output);
            }
        }
        
        // 输出测试点结果
        string result_str;
        switch (point.result) {
            case AC: result_str = "AC"; break;
            case WA: result_str = "WA"; break;
            case TLE: result_str = "TLE"; break;
            case MLE: result_str = "MLE"; break;
            case RE: result_str = "RE"; break;
            case UKE: result_str = "UKE"; break;
            default: result_str = "UKE";
        }
        
        cout << "测试点 " << point_name << ": " << result_str;
        if (point.result == AC) {
            cout << " (" << point.time_used << "ms, " 
                 << point.memory_used << "KB)";
            total_score += config.total_score * point.point_ratio / (double)total_ratio;
        }
        cout << endl;
        
        // 清理临时文件
        remove(student_output.c_str());
    }
    
    cout << endl;
    cout << "评测结束" << endl;
    cout << "总分: " << (int)total_score << "/" << config.total_score << endl;
    
    // 清理可执行文件
    remove("/tmp/student");
    if (config.special_judge) {
        remove("/tmp/checker");
    }
    
    return 0;
}
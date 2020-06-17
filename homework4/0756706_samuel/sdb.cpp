#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <capstone/capstone.h>
#include "elftool.h"

using namespace std;

class SDB
{
  public:
    SDB()
    {
        init();
    }

    SDB(string program)
    {
        init();
        load_prog({"", program});
    }

    ~SDB()
    {
        elf_close(eh);
        cs_close(&capstone_handle);
    }

    void init()
    {
        elf_init();
        cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
        current = State::BEGIN;
        text_idx = -1;
        eh = nullptr;
        tab = nullptr;
        target_pid = 0;
        base_addr = -1;
        last_disasm = -1;
        last_dump = -1;
        bp_idx = -1;
    }

    void run()
    {
        string input;
        vector<string> argv;
        bool ret;
        int status;
        cerr << "sdb> ";
        while (true)
        {
            getline(cin, input);
            argv = split(input, ' ');
            if (call.find(argv[0]) != call.end())
            {
                if (call[argv[0]] == &SDB::quit)
                    break;
                
                ret = (this->*call[argv[0]])(argv);
                if (ret)
                {
                    if (waitpid(target_pid, &status, 0) < 0)
                        cerr << "waitpid" << endl;
                    if (!WIFSTOPPED(status))
                        program_terminated(status);
                    else
                        check_if_bp();
                }
            }
            else
                cerr << "** Command not found." << endl;
            cerr << "sdb> ";
        }
    }

    bool load_prog(vector<string> argv)
    {
        if (current != State::BEGIN)
        {
            cerr << "** Program '" << eh->filename << "' has already been loaded." << endl;
            cerr << "** '" << eh->filename << "': entry point 0x" << hex << eh->entrypoint
                 << ", vaddr 0x" << eh->shdr[text_idx].addr
                 << ", offset 0x" << eh->shdr[text_idx].offset
                 << ", size 0x" << eh->shdr[text_idx].size << endl;
        }
        else if (argv.size() == 1)
            cerr << "** Filename not specified." << endl;
        else if ((eh = elf_open(argv[1].c_str())) == nullptr)
            cerr << "** Unable to open file." << endl;
        else
        {
            if (elf_load_all(eh) < 0)
                cerr << "** Unable to load ELF file." << endl;
            else
            {
                for (tab = eh->strtab; tab != nullptr; tab = tab->next)
                    if (tab->id == eh->shstrndx) break;
                if (tab == nullptr)
                    cerr << "** Section header string table not found." << endl;
                else
                {
                    for (int i = 0; i < eh->shnum; ++i)
                    {
                        if (strcmp(&tab->data[eh->shdr[i].name], ".text") == 0)
                        {
                            text_idx = i;
                            break;
                        }
                    }
                    if (text_idx == -1)
                        cerr << "** .text section not found." << endl;
                    else
                    {
                        cerr << "** Program '" << eh->filename << "' loaded. "
                             << "entry point 0x" << hex << eh->entrypoint
                             << ", vaddr 0x" << eh->shdr[text_idx].addr
                             << ", offset 0x" << eh->shdr[text_idx].offset
                             << ", size 0x" << eh->shdr[text_idx].size << endl;
                        current = State::LOADED;
                        return false;
                    }
                }
            }
            elf_close(eh);
            eh = nullptr;
            tab = nullptr;
        }
        return false;
    }

    bool set_bp(vector<string> argv)
    {
        long long addr = stoll(argv[1], nullptr, 0);
        for (auto& item : break_points)
        {
            if (item.addr == addr)
            {
                cerr << "** Break point has been set." << endl;
                return false;
            }
        }

        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
        {
            if (addr < eh->shdr[text_idx].addr || addr >= eh->shdr[text_idx].addr + eh->shdr[text_idx].size)
            {
                cerr << "** Address out of range. ("
                     << hex << eh->shdr[text_idx].addr << "-"
                     << eh->shdr[text_idx].addr + eh->shdr[text_idx].size - 1 << ")" << endl;
            }
            else
            {
                BP buf;
                buf.addr = addr;
                lseek(eh->fd, eh->shdr[text_idx].offset + addr - eh->shdr[text_idx].addr, SEEK_SET);
                read(eh->fd, &buf.original_byte, sizeof(buf.original_byte));
                break_points.push_back(buf);
            }
        }
        else
        {
            if (addr < eh->shdr[text_idx].addr + base_addr || addr > last_addr)
            {
                cerr << "** Address out of range. ("
                     << hex << eh->shdr[text_idx].addr + base_addr << "-" << last_addr << ")" << endl;
            }
            else
            {
                BP buf;
                buf.addr = addr;
                long ret = ptrace(PTRACE_PEEKTEXT, target_pid, addr, 0);
                memcpy(&buf.original_byte, &ret, 1);
                break_points.push_back(buf);
                if (ptrace(PTRACE_POKETEXT, target_pid, addr, (ret & 0xffffffffffffff00) | 0xcc) != 0)
                    cerr << "ptrace(POKETEXT)" << endl;
            }
        }
        return false;
    }

    bool delete_bp(vector<string> argv)
    {
        unsigned long idx = stoul(argv[1]);
        if (idx >= break_points.size())
            cerr << "** Break point not found." << endl;
        else
        {
            if (current == State::RUNNING)
            {
                long ret = ptrace(PTRACE_PEEKTEXT, target_pid, break_points[idx].addr, 0);
                ret = (ret & 0xffffffffffffff00) | break_points[idx].original_byte;
                if (ptrace(PTRACE_POKETEXT, target_pid, break_points[idx].addr, ret) != 0)
                    cerr << "ptrace(POKETEXT)" << endl;
                if (bp_idx == (int)idx)
                {
                    bp_idx = -1;
                    if (ptrace(PTRACE_SETREGS, target_pid, 0, &regs) != 0)
                        perror("ptrace(SETREGS)");
                }
            }
            break_points.erase(break_points.begin() + idx);
            cerr << "** breakpoint " << idx << " deleted." << endl;
        }
        return false;
    }

    bool list_bp(vector<string> argv)
    {
        for (size_t i = 0; i < break_points.size(); ++i)
        {
            cerr << dec << right << setw(3) << i << ": "
                 << hex << setw(16) << break_points[i].addr << " " << endl;
        }
        return false;
    }

    bool dis_asm(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else
        {
            if (argv.size() == 1 && last_disasm == -1)
                cerr << "** No addr is given" << endl;
            else
            {
                unsigned char code[113] = {0};
                long code_size = 0;
                long long start_addr = argv.size() == 1 ? last_disasm : stoll(argv[1], nullptr, 0);
                long long end_addr;
                if (current == State::LOADED)
                {
                    if (start_addr < eh->shdr[text_idx].addr || start_addr >= eh->shdr[text_idx].addr + eh->shdr[text_idx].size)
                    {
                        start_addr = -1;
                        cerr << "** Address out of range. ("
                             << hex << eh->shdr[text_idx].addr << "-"
                             << eh->shdr[text_idx].addr + eh->shdr[text_idx].size - 1 << ")" << endl;
                    }
                    else
                    {
                        size_t seek_pos = eh->shdr[text_idx].offset + start_addr - eh->shdr[text_idx].addr;
                        lseek(eh->fd, seek_pos, SEEK_SET);
                        read(eh->fd, code, sizeof(code) - 1);
                        end_addr = eh->shdr[text_idx].addr + eh->shdr[text_idx].size;
                    }
                }
                else
                {
                    // no need to check address
                    /*
                    if (start_addr < eh->shdr[text_idx].addr + base_addr || start_addr > last_addr)
                    {
                        start_addr = -1;
                        cerr << "** Address out of range. ("
                             << hex << eh->shdr[text_idx].addr + base_addr << "-" << last_addr  << ")" << endl;
                    }
                    else
                    {*/
                        long word;
                        for (int i = 0; i < 14; ++i)
                        {
                            errno = 0;
                            word = ptrace(PTRACE_PEEKTEXT, target_pid, start_addr + i * 8, 0);
                            if (errno != 0)
                            {
                                if (i == 0)
                                    cerr << "** Address out of range." << endl;
                                break;
                            }
                            memcpy(code + i * 8, &word, 8);
                            code_size += 8;
                        }
                        for (auto& item : break_points)
                        {
                            if (item.addr >= start_addr && item.addr < start_addr + code_size)
                                *(code + item.addr - start_addr) = item.original_byte;
                        }
                        end_addr = start_addr + code_size;
                    //}
                }

                if (start_addr != -1)
                {
                    size_t count;
                    cs_insn *insn;
                    count = cs_disasm(capstone_handle, code, sizeof(code) - 1, start_addr, 10, &insn);
                    if (count > 0)
                    {
                        for (size_t i = 0; i < count && start_addr < end_addr; ++i)
                        {
                            print_instruction(insn[i]);
                            start_addr += insn[i].size;
                        }
                        cs_free(insn, count);
                    }
                    else
                        cerr << "** Fail to disassmble address " << start_addr << "." << endl;
                    last_disasm = start_addr;
                }
            }
        }
        return false;
    }

    bool quit(vector<string> argv)
    {
        return true;
    }

    bool print_help(vector<string> argv)
    {
        cerr << "- break {instruction-address}: add a break point\n"
             << "- cont: continue execution\n"
             << "- delete {break-point-id}: remove a break point\n"
             << "- disasm addr: disassemble instructions in a file or a memory region\n"
             << "- dump addr [length]: dump memory content\n"
             << "- exit: terminate the debugger\n"
             << "- get reg: get a single value from a register\n"
             << "- getregs: show registers\n"
             << "- help: show this message\n"
             << "- list: list break points\n"
             << "- load {path/to/a/program}: load a program\n"
             << "- run: run the program\n"
             << "- vmmap: show memory layout\n"
             << "- set reg val: get a single value to a register\n"
             << "- si: step into instruction\n"
             << "- start: start the program and stop at the first instruction\n";
        return false;
    }

    bool print_mem_layout(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
        {
            cerr << setfill('0') << hex << right << setw(16) << eh->shdr[text_idx].addr << "-"
                 << setw(16) << eh->shdr[text_idx].addr + eh->shdr[text_idx].size << " r-x "
                 << setfill(' ') << left << setw(8) << eh->shdr[text_idx].offset << " "
                 << eh->filename << endl;
        }
        else
            parse_maps();
        return false;
    }

    bool exec(vector<string> argv)
    {
        return exec_impl(argv, true);
    }

    bool start_exec(vector<string> argv)
    {
        return exec_impl(argv, false);
    }

    bool continue_exec(vector<string> argv)
    {
        return continue_impl(argv, PTRACE_CONT);
    }

    bool run_a_instruction(vector<string> argv)
    {
        return continue_impl(argv, PTRACE_SINGLESTEP);
    }

    bool get_reg(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
            cerr << "** Program is not running." << endl;       
        else
        {
            if (reg_map.find(argv[1]) == reg_map.end())
                cerr << "** Register not found." << endl;
            else
            {
                unsigned long long int *regs_ptr = reinterpret_cast<unsigned long long int*>(&regs);
                cerr << argv[1] << " = " << dec << *(regs_ptr + reg_map[argv[1]]) << hex << " (0x" << *(regs_ptr + reg_map[argv[1]])<< ")" << endl;
            }
        }
        return false;
    }

    bool get_all_reg(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
            cerr << "** Program is not running." << endl;
        else
        {
            unsigned long long int *regs_ptr = reinterpret_cast<unsigned long long int*>(&regs);
            cerr << left;
            for (auto& item : reg_map)
                cerr << setw(8) << item.first << " = " << dec << *(regs_ptr + item.second) << hex << " (0x" << *(regs_ptr + item.second) << ")" << endl;
        }
        return false;
    }

    bool set_reg(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
            cerr << "** Program is not running." << endl;
        else
        {
            if (reg_map.find(argv[1]) == reg_map.end())
                cerr << "** Register not found." << endl;
            else
            {
                unsigned long long int *regs_ptr = reinterpret_cast<unsigned long long int*>(&regs);
                *(regs_ptr + reg_map[argv[1]]) = stoull(argv[2], nullptr, 0);
                if (ptrace(PTRACE_SETREGS, target_pid, 0, &regs) != 0)
                    perror("ptrace(SETREGS)");
            }
            if (argv[1] == "rip")
                check_if_bp();
        }
        return false;
    }

    bool dump_mem(vector<string> argv)
    {
        if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else if (current == State::LOADED)
            cerr << "** Program is not running." << endl;
        else
        {
            if (argv.size() == 1 && last_dump == -1)
                cerr << "** No addr is given" << endl;
            else
            {
                long data = -1;
                unsigned char *data_ptr = reinterpret_cast<unsigned char*>(&data);
                string show;
                long long start_addr = argv.size() == 1 ? last_dump : stoll(argv[1], nullptr, 0);
                for (int i = 0; i < 10; ++i)
                {
                    errno = 0;
                    data = ptrace(PTRACE_PEEKTEXT, target_pid, start_addr, 0);
                    if (i % 2 == 0)
                    {
                        if (errno != 0)
                        {
                            if (i == 0)
                                cerr << "** Address out of range." << endl;
                            break;
                        }
                        cerr << right << hex << setfill(' ') << setw(16) << start_addr << ": " << setfill('0');
                    }
                    for (int j = 0; j < 8; ++j)
                    {
                        if (errno == 0)
                        {
                            cerr << setw(2) << (int)*(data_ptr + j) << " ";
                            if (*(data_ptr + j) < 32 || *(data_ptr + j) > 126)
                                show.push_back('.');
                            else
                                show.push_back(*(data_ptr + j));
                        }
                        else
                        {
                            cerr << "   ";
                            show.push_back(' ');
                        }
                    }
                    if (i % 2 == 1)
                    {
                        cerr << " |" << show << "|" << endl;
                        show.clear();
                    }
                    start_addr += 8;
                }
                cerr << setfill(' ');
                last_dump = start_addr;
            }
        }
        return false;
    }

  private:
    vector<string> split(string target, char delim)
    {
        vector<string> result;
        for (size_t pos = target.find(delim); pos != string::npos; pos = target.find(delim))
        {
            result.emplace_back(target.substr(0, pos));
            while (target[pos + 1] == delim)
                ++pos;
            target = target.substr(pos + 1);
        }
        result.emplace_back(target);
        return result;
    }

    void print_instruction(cs_insn insn)
    {
        int i;
        cerr << hex << right << setw(16) << insn.address << ": " << setfill('0');
        for (i = 0; i < insn.size; ++i)
            cerr << setw(2) << (int)insn.bytes[i] << " ";
        for (; i < 10; ++i)
            cerr << "   ";
        cerr << " " << setfill(' ') << left << setw(6) << insn.mnemonic << " " << insn.op_str << endl;
    }

    void parse_maps(bool find_base_addr = false)
    {
        ifstream infile((string("/proc/") + to_string(target_pid) + "/maps").c_str());
        string line;
        size_t pos, rest2_pos;
        vector<string> rest;
        while (getline(infile, line))
        {
            pos = line.find('-');
            rest = split(line.substr(pos + 1), ' ');
            if (find_base_addr)
            {
                char cwd[256];
                getcwd(cwd, sizeof(cwd));
                if (rest[1].substr(0, rest[1].size() - 1) == "r-x" && rest[5] == string(cwd) + "/" + eh->filename)
                {
                    base_addr = stoll(line.substr(0, pos), nullptr, 16);
                    last_addr = stoll(rest[0], nullptr, 16);
                }
            }
            else
            {
                rest2_pos = 0;
                while (rest[2][rest2_pos] == '0' && rest2_pos != rest[2].size() - 1)
                    ++rest2_pos;
                cerr << setfill('0') << hex << right << setw(16) << line.substr(0, pos) << "-"
                     << setw(16) << rest[0] << " " << rest[1].substr(0, rest[1].size() - 1) << " "
                     << setfill(' ') << left << setw(8) << rest[2].substr(rest2_pos) << " "
                     << rest[5] << endl;
            }
        }
        infile.close();
    }

    bool exec_impl(vector<string> argv, bool cont)
    {
        if (current == State::BEGIN)
        {
            cerr << "** Program not loaded." << endl;
            return false;
        }
        else if (current == State::RUNNING)
            cerr << "** Program '" << eh->filename << "' is already running." << endl;
        else if ((target_pid = fork()) < 0)
            cerr << "fork()" << endl;
        else
        {
            if (target_pid == 0)
            {
                if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
                    cerr << "ptrace(TRACEME)" << endl;
                char* args[256] = {nullptr};
                for (size_t i = 1; i < argv.size(); ++i)
                    args[i - 1] = strdup(argv[i].c_str());
                char cwd[256];
                getcwd(cwd, sizeof(cwd));
                execvp(strcat(strcat(cwd, "/"), eh->filename), args);
                perror("execvp");
            }
            else
            {
                int status;
                if (waitpid(target_pid, &status, 0) < 0)
                    cerr << "waitpid" << endl;
                assert(WIFSTOPPED(status));
                ptrace(PTRACE_SETOPTIONS, target_pid, 0, PTRACE_O_EXITKILL);

                long ret;
                parse_maps(true);
                if (base_addr + eh->shdr[text_idx].offset == eh->shdr[text_idx].addr)
                    base_addr = 0;
                for (auto& item : break_points)
                {
                    item.addr += base_addr;
                    ret = ptrace(PTRACE_PEEKTEXT, target_pid, item.addr, 0);
                    if (ptrace(PTRACE_POKETEXT, target_pid, item.addr, (ret & 0xffffffffffffff00) | 0xcc) != 0)
                        cerr << "ptrace(POKETEXT)" << endl;
                }
                if (last_disasm != -1)
                    last_disasm += base_addr;
                cerr << dec << "** pid " << target_pid << endl;
                current = State::RUNNING;
            }
        }
        if (!cont)
        {
            check_if_bp();
            return false;
        }
        continue_impl(argv, PTRACE_CONT);
        return true;
    }

    bool continue_impl(vector<string> argv, __ptrace_request command)
    {
        if (current == State::RUNNING)
        {
            long ret;
            if (bp_idx != -1)
            {
                ret = ptrace(PTRACE_PEEKTEXT, target_pid, break_points[bp_idx].addr, 0);
                ret = (ret & 0xffffffffffffff00) | break_points[bp_idx].original_byte;
                if (ptrace(PTRACE_POKETEXT, target_pid, break_points[bp_idx].addr, ret) != 0)
                    perror("ptrace(POKETEXT)");
                if (ptrace(PTRACE_SETREGS, target_pid, 0, &regs) != 0)
                    perror("ptrace(SETREGS)");
            }
            ptrace(command, target_pid, 0, 0);
            if (bp_idx != -1)
            {
                ret = (ret & 0xffffffffffffff00) | (0x000000ff & 0xcc);
                if (ptrace(PTRACE_POKETEXT, target_pid, break_points[bp_idx].addr, ret) != 0 && errno != 3)
                    perror("ptrace(POKETEXT)");
            }
            return true;
        }
        else if (current == State::BEGIN)
            cerr << "** Program not loaded." << endl;
        else
            cerr << "** Program is not running." << endl;
        return false;
    }

    void program_terminated(int status)
    {
        if (WIFEXITED(status))
            cerr << dec << "** Child process " << target_pid << " terminated normally (code " << WEXITSTATUS(status) << ")." << endl;
        else if (WIFSIGNALED(status))
            cerr << dec << "** Child process " << target_pid << " killed by signal " << WSTOPSIG(status) << "." << endl;
        current = State::LOADED;
        target_pid = 0;
        for (auto& item : break_points)
            item.addr -= base_addr;
        if (last_disasm != -1)
            last_disasm -= base_addr;
        base_addr = -1;
        last_dump = -1;
        bp_idx = -1;
    }

    void check_if_bp()
    {
        if (ptrace(PTRACE_GETREGS, target_pid, 0, &regs) != 0)
            perror("ptrace(GETREGS)");
        
        bp_idx = -1;
        for (size_t i = 0;i < break_points.size(); ++i)
        {
            if (break_points[i].addr == (long long)regs.rip - 1)
            {
                bp_idx = i;
                break;
            }
        }

        if (bp_idx != -1)
        {
            unsigned char code[17];
            cs_insn *insn;
            regs.rip -= 1;

            long word;
            for (int i = 0; i < 2; ++i)
            {
                word = ptrace(PTRACE_PEEKTEXT, target_pid, regs.rip + i * 8, 0);
                memcpy(code + i * 8, &word, 8);
            }
            code[0] = break_points[bp_idx].original_byte;
            /*
            lseek(eh->fd, eh->shdr[text_idx].offset + regs.rip - eh->shdr[text_idx].addr - base_addr, SEEK_SET);
            read(eh->fd, code, sizeof(code) - 1);*/
            if (cs_disasm(capstone_handle, code, sizeof(code) - 1, regs.rip, 1, &insn) == 1)
            {
                cerr << "** Breakpoint @ ";
                print_instruction(insn[0]);
                cs_free(insn, 1);
            }
            else
                cerr << "** Fail to disassmble address " << regs.rip << "." << endl;
        }
    }

    using op_func_type = bool (SDB::*)(vector<string>);
    enum class State {BEGIN, LOADED, RUNNING};
    struct BP
    {
        long long addr;
        unsigned char original_byte;
    };
    
    map<string, op_func_type> call = {
        {"break", &SDB::set_bp},
        {"b", &SDB::set_bp},
        {"cont", &SDB::continue_exec},
        {"c", &SDB::continue_exec},
        {"delete", &SDB::delete_bp},
        {"disasm", &SDB::dis_asm},
        {"d", &SDB::dis_asm},
        {"dump", &SDB::dump_mem},
        {"x", &SDB::dump_mem},
        {"exit", &SDB::quit},
        {"q", &SDB::quit},
        {"get", &SDB::get_reg},
        {"g", &SDB::get_reg},
        {"getregs", &SDB::get_all_reg},
        {"help", &SDB::print_help},
        {"h", &SDB::print_help},
        {"list", &SDB::list_bp},
        {"l", &SDB::list_bp},
        {"load", &SDB::load_prog},
        {"run", &SDB::exec},
        {"r", &SDB::exec},
        {"vmmap", &SDB::print_mem_layout},
        {"m", &SDB::print_mem_layout},
        {"set", &SDB::set_reg},
        {"s", &SDB::set_reg},
        {"si", &SDB::run_a_instruction},
        {"start", &SDB::start_exec}
    };
    map<string, size_t> reg_map = {
        {"r15", 0},
        {"r14", 1},
        {"r13", 2},
        {"r12", 3},
        {"rbp", 4},
        {"rbx", 5},
        {"r11", 6},
        {"r10", 7},
        {"r9", 8},
        {"r8", 9},
        {"rax", 10},
        {"rcx", 11},
        {"rdx", 12},
        {"rsi", 13},
        {"rdi", 14},
        {"orig_rax", 15},
        {"rip", 16},
        {"cs", 17},
        {"eflags", 18},
        {"rsp", 19},
        {"ss", 20},
        {"fs_base", 21},
        {"gs_base", 22},
        {"ds", 23},
        {"es", 24},
        {"fs", 25},
        {"gs", 26}
    };

    State current;
    int text_idx;
    elf_handle_t *eh;
    elf_strtab_t *tab;
    csh capstone_handle;
    pid_t target_pid;
    vector<BP> break_points;
    long long base_addr;
    long long last_addr;
    long long last_disasm;
    long long last_dump;
    struct user_regs_struct regs;
    int bp_idx;
};

int main(int argc, char **argv)
{
    SDB* sdb;
    if (argc < 2)
        sdb = new SDB();
    else
        sdb = new SDB(argv[1]);
    sdb->run();
    delete sdb;
}

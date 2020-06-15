#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <capstone/capstone.h>

int main()
{
    // char tmpline[100];
    // char line[100];
    // std::FILE* sample_file = std::fopen("./sample.txt", "r");
    // fgets(tmpline, sizeof(tmpline), sample_file);
    // int i=0;
    // for(int idx=2; idx<strlen(tmpline)-3; ++idx)
    // {
    //     // std::cout << tmpline[idx];
    //     line[i++] = tmpline[idx];
    // }
    // line[i] = '\0';
    // std::cout << line;
    
    char CODE[] = {"H\xff\xcaH)\xfaH\xf7\xdeVH\x85\xd6"};
    char answer[1000];
    static csh handle = 0;
    cs_insn *insn;
    size_t count;

    cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
    count = cs_disasm(handle, (uint8_t*)CODE, sizeof(CODE) - 1, 0, 0, &insn);
    // std::cout << std::hex << int(insn[0].mnemonic[0]);
    // std::cout << strlen(insn[0].mnemonic);

    int cnt = 0;
    if (count > 0)
    {
        for ( size_t i = 0; i < count; ++i)
            std::cout << insn[i].mnemonic;
        std::cout << std::endl;
        
        for(size_t j=0; j<count; ++j)
        {
            for (size_t k=0; k<strlen(insn[j].mnemonic); ++k)
            {                
                std::cout << std::hex << int(insn[j].mnemonic[k]);
            }
            std::cout << "0a";
        }
        std::cout << std::endl;
        
        cs_free(insn, count);
    }
    else
    {
        printf("ERROR: Failed to disassemble given code!\n");
    }
    cs_close(&handle);

    // if( (count = cs_disasm(cshandle, (uint8_t*) buf, bufsz, rip, 0, &insn)) > 0 )
    // {
    //     int i;
    //     for (i = 0; i< count; ++i)
    //     {
    //         instruction in;
    //         in.size = insn[i].size;
    //         in.opr = insn[i].mnemonic;
    //         in.opnd = insn[i].op_str;
    //         memcpy(in.bytes, insn[i].bytes, insn[i].size);
    //     }
    //     cs_free(insn, count);
    // }
    // cs_close(&cshandle);

    return 0;
}
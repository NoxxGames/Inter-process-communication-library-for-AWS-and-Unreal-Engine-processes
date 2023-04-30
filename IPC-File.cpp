
#include <cstdlib>

#include "IPCFile.h"

int main(int argc, char* argv[])
{

    const IPCFile::FAttribute<int> myat = IPCFile::FAttribute<int>(IPCFile::EAttributeTypes::INT, 5);
    
    system("pause");
    return 0;
}


#include <cstdlib>

#include "IPCFile.h"

int main(int argc, char* argv[])
{

    const IPCFile::IAttribute<int> myat = IPCFile::IAttribute<int>(IPCFile::EAttributeTypes::INT, IPCFile::EAttributeName::PLAYER_NAME, 5);
    
    system("pause");
    return 0;
}

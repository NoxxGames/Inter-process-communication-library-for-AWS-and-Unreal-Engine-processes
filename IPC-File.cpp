#include "IPCFile.h"

using namespace IPCFile;

int main(int argc, char* argv[])
{
    const IAttributeString PlayerAuth = IAttributeString(
        EAttributeName::IS_ONLINE, "TestValue");
    std::vector<EAttributeName> AttributesToGet;
    AttributesToGet.push_back(EAttributeName::IS_ONLINE);
    FGetRequest GetRequest = FGetRequest(PlayerAuth, AttributesToGet);

    IPCFileManager::UE_Initialize([=]()
    {
        // dont need this for this test
    });

    IPCFileManager::UE_AddGetRequestToBuffer(GetRequest);
    IPCFileManager::UE_WriteGetRequestBufferToFile(
        "C:\\Users\\josh_\\Documents\\IPCtest");
    
    system("pause");
    return 0;
}
